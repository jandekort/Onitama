//
// Created by Jan de Kort on 2019-08-18.
//

#include "NodeVisitor.h"
#include "Node.h"
#include "TranspositionTable.h"
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <utility>

namespace {

// Evaluation weights, grouped so they are easy to find and tune.
constexpr double STUDENT_VALUE   = 10.0; // material: each student
constexpr double STREAM_WEIGHT   = 0.5;  // Way of the Stream: master advance
constexpr double CENTER_BONUS    = 0.5;  // occupying the central 3x3
constexpr double IMBALANCE_BONUS = 3.0;  // per extra student

// Evaluate the position from the perspective of one player (relative to
// opponent). Positive = good for `perspective`. One pass over the pieces fills
// per-color accumulators (indexed by Piece::Color: Blue=0, Red=1).
double evaluatePosition(const Node& n, Piece::Color perspective)
{
    double colorScore[2] = {0.0, 0.0};
    int students[2] = {0, 0};

    for (const auto& p : n.m_piece) {
        int c = p.mColor;
        int x = p.mPosition.mX;
        int y = p.mPosition.mY;
        double& s = colorScore[c];

        if (p.mIsMaster) {
            // Way of the Stream: reward the master for closing in on the
            // enemy temple arch (squared, so the last steps matter most).
            Position enemyTemple = Node::temple(Piece::other(p.mColor));
            int dist = std::max(std::abs(x - enemyTemple.mX),
                                std::abs(y - enemyTemple.mY));
            s += STREAM_WEIGHT * (4 - dist) * (4 - dist);
        } else {
            s += STUDENT_VALUE; // material: each student
            students[c]++;
        }

        // Center control: holding the central 3x3 is valuable, the very
        // centre square doubly so.
        if (x >= 1 && x <= 3 && y >= 1 && y <= 3) {
            s += CENTER_BONUS;
            if (x == 2 && y == 2) s += CENTER_BONUS;
        }
    }

    int me = perspective, opp = Piece::other(perspective);
    double imbalance = IMBALANCE_BONUS * (students[me] - students[opp]);
    return (colorScore[me] - colorScore[opp]) + imbalance;
}

// Order moves so captures (especially of the master) are searched first.
// Good moves first means alpha-beta gets its cut-offs early and prunes far
// more of the tree -- the single biggest speed-up available to this search.
// The sort is over a handful of moves, so its cost is tiny next to the
// sub-tree it saves.
void orderMoves(const Node& n, std::vector<Ply>& moves)
{
    auto key = [&n](const Ply& ply) -> int {
        if (ply.mIsPass) return -1;
        const Piece* victim = n.pieceAt(ply.mTo);
        if (!victim) return 0;                 // quiet move
        return victim->mIsMaster ? 1000 : 100; // master capture >> student
    };
    std::stable_sort(moves.begin(), moves.end(),
                     [&](const Ply& a, const Ply& b) { return key(a) > key(b); });
}

bool samePly(const Ply& a, const Ply& b)
{
    return a.mIsPass == b.mIsPass && a.mCardSlot == b.mCardSlot &&
           a.mFrom == b.mFrom && a.mTo == b.mTo;
}

// Move a transposition-table suggestion to the front so it is tried first,
// keeping the relative order of the rest. This only reorders the search; it
// never changes which moves are legal, so the value computed is unaffected.
void bringToFront(std::vector<Ply>& moves, const Ply& first)
{
    auto it = std::find_if(moves.begin(), moves.end(),
                           [&](const Ply& p) { return samePly(p, first); });
    if (it != moves.end() && it != moves.begin())
        std::rotate(moves.begin(), it, it + 1);
}

// Mate scores carry the distance to mate in their magnitude (WIN + remaining
// depth), so the same position scored at a different remaining depth gets a
// different number. Before caching a mate score we strip the depth out, and we
// add it back on retrieval, so an entry stays valid no matter what depth it is
// reused at. Ordinary evaluations (magnitude well below WIN) pass through.
// The zone boundary sits far above any static eval and far below WIN.
double mateZone() { return BestMove::WIN / 2.0; }

double toCacheScore(double value, int depth)
{
    if (value >= mateZone()) return value - depth;
    if (value <= -mateZone()) return value + depth;
    return value;
}

double fromCacheScore(double value, int depth)
{
    if (value >= mateZone()) return value + depth;
    if (value <= -mateZone()) return value - depth;
    return value;
}

} // namespace

void Score::visit(Node &n)
{
    n.m_score = evaluatePosition(n, n.m_tomove);
}

const double BestMove::WIN = 100000.0;

double BestMove::negamax(const Node& n, int depth, double alpha, double beta,
                         std::vector<Ply>& pv)
{
    mNodes++;
    pv.clear(); // a leaf/loss contributes no continuation

    // Time budget: poll the clock occasionally (cheap: once every 4096 nodes)
    // and, once past the deadline, unwind the whole pass. The aborted pass is
    // discarded by the caller, so returning any value here is harmless.
    if (mUseDeadline && (mNodes & 0xFFF) == 0 &&
        std::chrono::steady_clock::now() >= mDeadline)
        mAborted = true;
    if (mAborted)
        return 0.0;

    // The side to move may already have lost (master captured or enemy
    // master on our temple). Deeper remaining depth = earlier mate, so
    // adding `depth` makes the search prefer the quickest win.
    if (n.isLoss())
        return -(WIN + depth);

    if (depth <= 0) {
        // Static eval is read-only, so score `n` in place: copying a Node per
        // leaf (the hottest path in the search) was pure overhead.
        return evaluatePosition(n, n.m_tomove);
    }

    // --- Transposition table probe. Everything in this block is skipped when
    // the table is disabled, so the plain search is exactly as before.
    const double alphaOrig = alpha;
    std::uint64_t ttKey = 0;
    Ply ttMove;
    bool haveTtMove = false;
    if (mTT) {
        ttKey = mTT->hash(n);
        TTEntry e;
        if (mTT->probe(ttKey, e)) {
            if (e.hasMove) { ttMove = e.move; haveTtMove = true; }
            // Only trust an entry searched at least as deep as we need.
            if (e.depth >= depth) {
                double v = fromCacheScore(e.value, depth);
                bool usable = e.bound == TTBound::Exact ||
                              (e.bound == TTBound::Lower && v >= beta) ||
                              (e.bound == TTBound::Upper && v <= alpha);
                if (usable) {
                    if (e.hasMove) pv.push_back(e.move); // best-effort PV head
                    return v;
                }
            }
        }
    }

    double best = -2.0 * WIN;
    Ply bestMove;
    bool haveBestMove = false;
    std::vector<Ply> childPv;

    std::vector<Ply> moves = n.legalMoves();
    orderMoves(n, moves);
    if (haveTtMove) bringToFront(moves, ttMove); // try the cached move first

    for (const auto& ply : moves) {
        Node child = n.applyPly(ply);
        double value = -negamax(child, depth - 1, -beta, -alpha, childPv);
        if (mAborted)
            return best; // time ran out mid-pass; skip the TT store below
        if (value > best) {
            best = value;
            bestMove = ply;
            haveBestMove = true;
            // This move becomes the line's head; splice the child's PV after it.
            pv.clear();
            pv.push_back(ply);
            pv.insert(pv.end(), childPv.begin(), childPv.end());
        }
        // Pruning is opt-in: pure minimax leaves the window untouched and
        // searches every move. The best value found is identical either way.
        if (mConfig.useAlphaBeta) {
            if (value > alpha) alpha = value;
            if (alpha >= beta) break; // beta cut-off
        }
    }

    // --- Transposition table store. The bound flag records what the window
    // told us: a value that never beat alpha is only an upper bound, one that
    // caused a cut-off is only a lower bound, otherwise it is exact.
    if (mTT) {
        TTBound bound = best <= alphaOrig ? TTBound::Upper
                      : best >= beta      ? TTBound::Lower
                      :                     TTBound::Exact;
        mTT->store(ttKey, depth, toCacheScore(best, depth), bound,
                   bestMove, haveBestMove);
    }

    return best;
}

BestMove::BestMove(int aDepth, int aMultiPV, SearchConfig aConfig)
    : mHasMove(false), mNodes(0), mDepthReached(0), mTimedOut(false),
      mDepth(aDepth), mMultiPV(aMultiPV), mConfig(aConfig),
      mUseDeadline(false), mAborted(false)
{
}

// Out-of-line so TranspositionTable only needs to be a complete type here,
// where unique_ptr<TranspositionTable>'s destructor is instantiated.
BestMove::~BestMove() = default;

void BestMove::visit(Node &n)
{
    mNodes = 0;
    mHasMove = false;
    mLines.clear();
    mDepthReached = 0;
    mTimedOut = false;
    mAborted = false;
    mUseDeadline = false;

    // TT and iterative deepening build on alpha-beta bounds, so they only
    // engage when pruning is on (pure minimax ignores both).
    const bool useTT = mConfig.useAlphaBeta && mConfig.useTranspositionTable;
    const bool useID = mConfig.useAlphaBeta && mConfig.useIterativeDeepening;

    // Build a fresh transposition table for this search if enabled, so results
    // never leak between separate searches. Iterative deepening reuses it
    // across its passes; a single fixed-depth search uses it within the tree.
    if (useTT)
        mTT.reset(new TranspositionTable(mConfig.ttEntries));
    else
        mTT.reset();

    // A time budget only applies with iterative deepening (it needs a completed
    // shallower pass to fall back on). The clock starts now.
    const bool timed = useID && mConfig.timeBudgetMs > 0;
    if (timed)
        mDeadline = std::chrono::steady_clock::now() +
                    std::chrono::milliseconds(mConfig.timeBudgetMs);

    if (useID) {
        // Search shallow first; each completed pass overwrites the previous and
        // primes the table. With a time budget, a pass cut short mid-way is
        // discarded (searchRoot commits nothing on abort), so mLines/mBest keep
        // the last fully completed depth.
        for (int d = 1; d <= mDepth; ++d) {
            // Always finish depth 1 so we never end up without a move; only
            // deeper passes may be stopped by the clock.
            mUseDeadline = timed && d > 1;
            searchRoot(n, d);
            if (mAborted)
                break;             // out of time; keep depth d-1's result
            mDepthReached = d;
        }
        mTimedOut = mAborted;      // stopped before finishing the depth ceiling
    } else {
        searchRoot(n, mDepth);
        if (mHasMove)
            mDepthReached = mDepth;
    }
}

void BestMove::searchRoot(Node &n, int depth)
{
    // Build into locals and commit only at the end. If the time budget aborts
    // this pass, we return early without touching mLines/mBest/mHasMove, so the
    // previous completed depth's result is what callers see.
    const double beta = 2.0 * WIN;
    double alpha = -2.0 * WIN;

    std::vector<ScoredLine> all;
    std::vector<Ply> childPv;

    std::vector<Ply> moves = n.legalMoves();
    orderMoves(n, moves);
    for (const auto& ply : moves) {
        // Skip root moves the caller asked to exclude (multi-line enumeration).
        if (!mExcludeRoot.empty() &&
            std::any_of(mExcludeRoot.begin(), mExcludeRoot.end(),
                        [&](const Ply& e) { return samePly(e, ply); }))
            continue;

        Node child = n.applyPly(ply);
        // Ordinary alpha-beta: narrow alpha as we go so beta cut-offs prune
        // unpromising siblings. This keeps the cost the same as a single-PV
        // search; the trade-off is that only the eventual best line gets an
        // exact score — the other kept lines carry the (bounded) value the
        // pruned search reached, which is what the engine actually computed.
        double value = -negamax(child, depth - 1, -beta, -alpha, childPv);
        if (mAborted) return; // pass incomplete; do not commit partial results

        ScoredLine sl;
        sl.score = value;
        sl.line.reserve(childPv.size() + 1);
        sl.line.push_back(ply);
        sl.line.insert(sl.line.end(), childPv.begin(), childPv.end());
        all.push_back(std::move(sl));

        if (value > alpha) alpha = value;
    }

    if (all.empty()) return;

    // Best line first. Only the top score is exact; the rest are upper bounds
    // <= it, so the true best still sorts to the front.
    std::stable_sort(all.begin(), all.end(),
                     [](const ScoredLine& a, const ScoredLine& b) {
                         return a.score > b.score;
                     });

    int keep = mMultiPV < 1 ? 1 : mMultiPV;
    if (static_cast<int>(all.size()) > keep)
        all.resize(keep);

    // Commit the completed pass.
    mLines = std::move(all);
    mHasMove = true;
    mBest = mLines.front().line.front();
    n.m_score = mLines.front().score;
}
