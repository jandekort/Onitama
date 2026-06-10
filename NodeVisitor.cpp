//
// Created by Jan de Kort on 2019-08-18.
//

#include "NodeVisitor.h"
#include "Node.h"
#include <algorithm>
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

    double best = -2.0 * WIN;
    std::vector<Ply> childPv;

    std::vector<Ply> moves = n.legalMoves();
    orderMoves(n, moves);
    for (const auto& ply : moves) {
        Node child = n.applyPly(ply);
        double value = -negamax(child, depth - 1, -beta, -alpha, childPv);
        if (value > best) {
            best = value;
            // This move becomes the line's head; splice the child's PV after it.
            pv.clear();
            pv.push_back(ply);
            pv.insert(pv.end(), childPv.begin(), childPv.end());
        }
        if (value > alpha) alpha = value;
        if (alpha >= beta) break; // beta cut-off
    }

    return best;
}

void BestMove::visit(Node &n)
{
    mNodes = 0;
    mHasMove = false;
    mLines.clear();

    const double beta = 2.0 * WIN;
    double alpha = -2.0 * WIN;

    std::vector<ScoredLine> all;
    std::vector<Ply> childPv;

    std::vector<Ply> moves = n.legalMoves();
    orderMoves(n, moves);
    for (const auto& ply : moves) {
        Node child = n.applyPly(ply);
        // Ordinary alpha-beta: narrow alpha as we go so beta cut-offs prune
        // unpromising siblings. This keeps the cost the same as a single-PV
        // search; the trade-off is that only the eventual best line gets an
        // exact score — the other kept lines carry the (bounded) value the
        // pruned search reached, which is what the engine actually computed.
        double value = -negamax(child, mDepth - 1, -beta, -alpha, childPv);

        ScoredLine sl;
        sl.score = value;
        sl.line.reserve(childPv.size() + 1);
        sl.line.push_back(ply);
        sl.line.insert(sl.line.end(), childPv.begin(), childPv.end());
        all.push_back(std::move(sl));

        if (value > alpha) alpha = value;
        mHasMove = true;
    }

    if (!mHasMove) return;

    // Best line first. Only the top score is exact; the rest are upper bounds
    // <= it, so the true best still sorts to the front.
    std::stable_sort(all.begin(), all.end(),
                     [](const ScoredLine& a, const ScoredLine& b) {
                         return a.score > b.score;
                     });

    int keep = mMultiPV < 1 ? 1 : mMultiPV;
    if (static_cast<int>(all.size()) > keep)
        all.resize(keep);
    mLines = std::move(all);

    mBest = mLines.front().line.front();
    n.m_score = mLines.front().score;
}
