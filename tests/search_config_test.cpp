// Equivalence tests for the optional search optimizations.
//
// The whole promise of SearchConfig is that the transposition table and
// iterative deepening are *correctness-neutral*: they change how much work the
// search does, never the answer it gives. These parameterized tests run the
// same positions under every combination of the flags and assert the
// decision-relevant outputs match the plain alpha-beta baseline exactly:
//
//   * mHasMove                 -- a move was (or was not) found
//   * mBest                    -- the chosen move, field for field
//   * mLines.size()            -- how many lines were kept
//   * mLines.front().score     -- the best line's exact value
//
// Why those four are invariant: root moves are searched in a config-independent
// order, so the first truly-best move in that order always records its exact
// value (the table preserves exact in-window values), every earlier move
// records strictly less, and the stable sort therefore puts the same move with
// the same exact score at the front under every config.
//
// Three things are deliberately NOT asserted equal, because they legitimately
// differ and that difference is either harmless or the whole point:
//   * mNodes                 -- the optimizations exist to shrink this
//   * non-best line scores   -- those are fail-soft bounds, not exact values
//   * the best line's PV tail -- a transposition cut-off can truncate the
//                               reconstructed continuation (the move and score
//                               are still exact; only the displayed line is shorter)
//
// The baseline (SearchConfig::plain()) is the reference; every other config is
// compared against it on the same Node.

#include <gtest/gtest.h>

#include "Node.h"
#include "NodeVisitor.h"
#include "SearchConfig.h"
#include "test_helpers.h"

namespace {

bool plyEqual(const Ply& a, const Ply& b)
{
    return a.mIsPass == b.mIsPass && a.mCardSlot == b.mCardSlot &&
           a.mFrom == b.mFrom && a.mTo == b.mTo;
}

// A position to search, with the depth the existing suite uses for it.
struct Case
{
    const char* name;
    const char* hash;
    int depth;
};

// Reuse the puzzle positions the engine tests already trust, across a spread
// of depths, plus the standard opening (a non-mate, branchy position) so the
// equivalence also covers ordinary evaluations, not just forced mates.
const Case kCases[] = {
    {"Puzzle1_d3", "AAAQQgYQAAAQcMtp", 3},
    {"Puzzle2_d5", "AABwCAAAAAhAaLtP", 5},
    {"Puzzle3_d5", "5AgBAABCgAAEKs5w", 5},
    {"Puzzle4_d5", "AAAQxAgEACAAGsIv", 5},
    {"Puzzle5_d7", "GAAABAAACCAAeEpn", 7},
    {"Puzzle6_d7", "AACAWAAAACABqBRA", 7},
    {"Opening_d5", "",                 5}, // empty hash -> standard start
};

Node startingPosition()
{
    Node node;
    node.m_tomove = Piece::Blue;
    for (int x = 0; x < 5; x++) {
        node.addPiece(Piece(Position(x, 0), Piece::Red,  x == 2));
        node.addPiece(Piece(Position(x, 4), Piece::Blue, x == 2));
    }
    return node;
}

Node positionFor(const Case& c)
{
    return std::string(c.hash).empty() ? startingPosition()
                                       : test::decodePosition(c.hash);
}

// One scenario = a case position crossed with a non-plain config to check.
struct Scenario
{
    Case position;
    SearchConfig config;
    const char* configName;
};

class SearchEquivalence : public ::testing::TestWithParam<Scenario> {};

TEST_P(SearchEquivalence, MatchesPlainAlphaBeta)
{
    const Scenario& s = GetParam();
    const int multiPV = 3; // exercise multi-line bookkeeping too

    // Baseline: plain alpha-beta, no optimizations.
    Node base = positionFor(s.position);
    BestMove baseline(s.position.depth, multiPV, SearchConfig::plain());
    base.accept(baseline);

    // Candidate: same position and depth, optimization(s) enabled.
    Node cand = positionFor(s.position);
    BestMove optimized(s.position.depth, multiPV, s.config);
    cand.accept(optimized);

    // Decision-relevant outputs must be identical.
    EXPECT_EQ(baseline.mHasMove, optimized.mHasMove);
    ASSERT_TRUE(optimized.mHasMove);
    EXPECT_TRUE(plyEqual(baseline.mBest, optimized.mBest))
        << "best move differs under " << s.configName;

    ASSERT_EQ(baseline.mLines.size(), optimized.mLines.size());
    EXPECT_DOUBLE_EQ(baseline.mLines.front().score,
                     optimized.mLines.front().score)
        << "best score differs under " << s.configName;
}

std::vector<Scenario> makeScenarios()
{
    const struct { SearchConfig cfg; const char* name; } configs[] = {
        {SearchConfig::withTT(), "TT"},
        {SearchConfig::withID(), "ID"},
        {SearchConfig::all(),    "TT_ID"},
    };

    std::vector<Scenario> out;
    for (const auto& c : kCases)
        for (const auto& cfg : configs)
            out.push_back(Scenario{c, cfg.cfg, cfg.name});
    return out;
}

std::string scenarioName(const ::testing::TestParamInfo<Scenario>& info)
{
    return std::string(info.param.position.name) + "_" + info.param.configName;
}

INSTANTIATE_TEST_SUITE_P(AllConfigs, SearchEquivalence,
                         ::testing::ValuesIn(makeScenarios()),
                         scenarioName);

} // namespace

// A direct check that iterative deepening's final pass equals a one-shot search
// of the same depth -- the property the ID loop relies on.
TEST(SearchConfig, IterativeDeepeningFinalPassMatchesDirect)
{
    Node a = test::decodePosition("AABwCAAAAAhAaLtP");
    Node b = test::decodePosition("AABwCAAAAAhAaLtP");

    BestMove direct(5, 1, SearchConfig::plain());
    a.accept(direct);

    BestMove iterative(5, 1, SearchConfig::withID());
    b.accept(iterative);

    ASSERT_TRUE(direct.mHasMove);
    ASSERT_TRUE(iterative.mHasMove);
    EXPECT_DOUBLE_EQ(direct.mLines.front().score,
                     iterative.mLines.front().score);
}

// The transposition table must actually save work, otherwise it is pointless.
// On a branchy enough search it should visit strictly fewer nodes than plain
// alpha-beta while returning the same score.
TEST(SearchConfig, TranspositionTableReducesNodes)
{
    Node a = test::decodePosition("AAAQACAsAAAAYMkF"); // Puzzle9, deep
    Node b = test::decodePosition("AAAQACAsAAAAYMkF");

    BestMove plain(11, 1, SearchConfig::plain());
    a.accept(plain);

    BestMove tt(11, 1, SearchConfig::withTT());
    b.accept(tt);

    EXPECT_DOUBLE_EQ(plain.mLines.front().score, tt.mLines.front().score);
    EXPECT_LT(tt.mNodes, plain.mNodes);
}

// Pure minimax (no pruning) is the ground truth the whole alpha-beta family
// must reproduce: same best move, same best score. Kept shallow because without
// pruning the tree explodes with depth. It must also visit strictly more nodes
// than the fully optimized search -- that gap is exactly what pruning buys.
TEST(SearchConfig, MinimaxAgreesWithAlphaBetaFamily)
{
    const int depth = 5;
    const char* hashes[] = {"AAAQQgYQAAAQcMtp", "AABwCAAAAAhAaLtP", ""};

    for (const char* h : hashes) {
        const bool opening = std::string(h).empty();
        Node ground = opening ? startingPosition() : test::decodePosition(h);
        Node tuned  = opening ? startingPosition() : test::decodePosition(h);

        BestMove minimax(depth, 3, SearchConfig::minimax());
        ground.accept(minimax);

        BestMove tuned_search(depth, 3, SearchConfig::all());
        tuned.accept(tuned_search);

        ASSERT_TRUE(minimax.mHasMove);
        ASSERT_TRUE(tuned_search.mHasMove);
        EXPECT_TRUE(plyEqual(minimax.mBest, tuned_search.mBest));
        EXPECT_DOUBLE_EQ(minimax.mLines.front().score,
                         tuned_search.mLines.front().score);
        EXPECT_GT(minimax.mNodes, tuned_search.mNodes); // no pruning costs nodes
    }
}

// A generous time budget never changes the answer: the search finishes the full
// depth, reports it did not time out, and matches the unbounded search exactly.
TEST(SearchConfig, GenerousTimeBudgetMatchesUnbounded)
{
    Node a = test::decodePosition("AABwCAAAAAhAaLtP");
    Node b = test::decodePosition("AABwCAAAAAhAaLtP");

    BestMove unbounded(7, 3, SearchConfig::all());
    a.accept(unbounded);

    SearchConfig cfg = SearchConfig::all();
    cfg.timeBudgetMs = 60000; // far more than this search needs
    BestMove timed(7, 3, cfg);
    b.accept(timed);

    ASSERT_TRUE(timed.mHasMove);
    EXPECT_FALSE(timed.mTimedOut);
    EXPECT_EQ(timed.mDepthReached, 7);
    EXPECT_TRUE(plyEqual(unbounded.mBest, timed.mBest));
    EXPECT_DOUBLE_EQ(unbounded.mLines.front().score, timed.mLines.front().score);
}

// A tiny budget cannot finish a deep search, but it must still return a legal
// move (depth 1 always completes), report the shallow depth it reached, and
// flag that it timed out.
TEST(SearchConfig, TinyTimeBudgetStillReturnsALegalMove)
{
    Node n = startingPosition();

    SearchConfig cfg = SearchConfig::all();
    cfg.timeBudgetMs = 1; // 1 ms: nowhere near enough for depth 15
    BestMove timed(15, 1, cfg);
    n.accept(timed);

    ASSERT_TRUE(timed.mHasMove);
    EXPECT_GE(timed.mDepthReached, 1);
    EXPECT_LT(timed.mDepthReached, 15);
    EXPECT_TRUE(timed.mTimedOut);

    std::vector<Ply> legal = n.legalMoves();
    bool isLegal = false;
    for (const auto& m : legal)
        if (plyEqual(m, timed.mBest)) isLegal = true;
    EXPECT_TRUE(isLegal);
}

// Root-move exclusion drives iterative multi-PV: excluding the best move yields
// the next-best, whose value is <= the best's. Verified against a full scan over
// the remaining root moves so the exclusion search really returns the 2nd best.
TEST(SearchConfig, RootExclusionYieldsNextBest)
{
    const int depth = 7;
    Node n = test::decodePosition("5EgAAAAAACBFIg8B"); // win-in-4 from the opening

    BestMove best(depth, 1, SearchConfig::all());
    n.accept(best);
    ASSERT_TRUE(best.mHasMove);

    // Second best = best after excluding the top move.
    Node n2 = test::decodePosition("5EgAAAAAACBFIg8B");
    BestMove second(depth, 1, SearchConfig::all());
    second.mExcludeRoot.push_back(best.mBest);
    n2.accept(second);
    ASSERT_TRUE(second.mHasMove);

    EXPECT_FALSE(plyEqual(best.mBest, second.mBest));        // a different move
    EXPECT_LE(second.mLines.front().score,
              best.mLines.front().score);                    // not better than #1

    // Independently: the true 2nd-best = max value over every root move except
    // the best, via a full per-child scan. The exclusion search must match it.
    double trueSecond = -2.0 * BestMove::WIN;
    std::vector<Ply> legal = n.legalMoves();
    for (const auto& m : legal) {
        if (plyEqual(m, best.mBest)) continue;
        Node child = n.applyPly(m);
        BestMove cs(depth - 1, 1, SearchConfig::all());
        child.accept(cs);
        double v = cs.mHasMove ? -cs.mLines.front().score : -2.0 * BestMove::WIN;
        if (v > trueSecond) trueSecond = v;
    }
    EXPECT_DOUBLE_EQ(second.mLines.front().score, trueSecond);
}

// Excluding every legal root move leaves nothing to search.
TEST(SearchConfig, ExcludingAllRootMovesYieldsNoMove)
{
    Node n = test::decodePosition("5EgAAAAAACBFIg8B");
    BestMove search(5, 1, SearchConfig::all());
    search.mExcludeRoot = n.legalMoves();
    n.accept(search);
    EXPECT_FALSE(search.mHasMove);
    EXPECT_TRUE(search.mLines.empty());
}
