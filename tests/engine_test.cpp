#include <gtest/gtest.h>

#include "Node.h"
#include "NodeVisitor.h"
#include "test_helpers.h"

namespace {

// Build the standard Onitama starting array: five pieces on each back rank,
// masters on the centre file. Cards are left at their default (all Tiger),
Node startingPosition(Piece::Color toMove = Piece::Blue)
{
    Node node;
    node.m_tomove = toMove;
    for (int x = 0; x < 5; x++) {
        node.addPiece(Piece(Position(x, 0), Piece::Red,  x == 2)); // top row
        node.addPiece(Piece(Position(x, 4), Piece::Blue, x == 2)); // bottom row
    }
    return node;
}

} // namespace

TEST(Engine, CapturingMasterIsLoss)
{
    Node node;
    node.m_tomove = Piece::Red;
    node.addPiece(Piece(test::sq("c5"), Piece::Red, true));   // Red master at c5
    node.addPiece(Piece(test::sq("e1"), Piece::Blue, true));  // Blue master at e1
    node.addPiece(Piece(test::sq("e2"), Piece::Red, false));  // Red pawn at e2

    ASSERT_FALSE(node.isLoss());  // Red to move, both masters present

    // Red pawn captures Blue master
    Ply ply;
    ply.mCardSlot = 0;
    ply.mIsPass = false;
    ply.mFrom = test::sq("e2");
    ply.mTo = test::sq("e1");
    Node after = node.applyPly(ply);
    EXPECT_TRUE(after.isLoss());  // Blue to move next, Blue's master is gone
}

TEST(Engine, EnemyMasterOnTempleIsLoss) {
    Node node;
    node.m_tomove = Piece::Blue;
    node.addPiece(Piece(test::sq("c1"), Piece::Red, true));   // Red master on Blue's temple
    node.addPiece(Piece(test::sq("c5"), Piece::Blue, true));  // Blue master at c5

    EXPECT_TRUE(node.isLoss());
}

TEST(Engine, ApplyPlySwapsToMove)
{
    Node node = startingPosition(Piece::Blue);
    EXPECT_EQ(node.m_tomove, Piece::Blue);

    std::vector<Ply> moves = node.legalMoves();
    ASSERT_FALSE(moves.empty());

    Node after = node.applyPly(moves[0]);
    EXPECT_EQ(after.m_tomove, Piece::Red);
}

TEST(Engine, Puzzle1_BlueWinIn2)
{
    // Load position from hash: Blue to move, wins in 2
    Node node = test::decodePosition("BAgK4AAAgCBEAKYJ");

    BestMove search(3);
    node.accept(search);

    ASSERT_TRUE(search.mHasMove);
    ASSERT_FALSE(search.mLines.empty());

    double score = search.mLines.front().score;
    ASSERT_GE(score, BestMove::WIN);

    int mateIn = test::matePlies(score, 3);
    EXPECT_EQ(mateIn, 3); // 2 plies from blue, 1 ply from red
}


TEST(Engine, Puzzle2_BlueWinIn3)
{
    // Load position from hash: Blue to move, wins in 3
    Node node = test::decodePosition("AABwCAAAAAhAaLtP");

    BestMove search(5);
    node.accept(search);

    ASSERT_TRUE(search.mHasMove);
    ASSERT_FALSE(search.mLines.empty());

    double score = search.mLines.front().score;
    ASSERT_GE(score, BestMove::WIN);

    int mateIn = test::matePlies(score, 5);
    EXPECT_EQ(mateIn, 5); // 3 plies from blue, 2 ply from red
}


TEST(Engine, Puzzle3_BlueWinIn3)
{
    // Load position from hash: Blue to move, wins in 3
    Node node = test::decodePosition("5AgBAABCgAAEKs5w");

    BestMove search(5);
    node.accept(search);

    ASSERT_TRUE(search.mHasMove);
    ASSERT_FALSE(search.mLines.empty());

    double score = search.mLines.front().score;
    ASSERT_GE(score, BestMove::WIN);

    int mateIn = test::matePlies(score, 5);
    EXPECT_EQ(mateIn, 5); // 3 plies from blue, 2 plies from red
}


TEST(Engine, Puzzle4_BlueWinIn3)
{
    // Load position from hash: Blue to move, wins in 3
    Node node = test::decodePosition("AICAIEAMAABCINcu");

    BestMove search(5);
    node.accept(search);

    ASSERT_TRUE(search.mHasMove);
    ASSERT_FALSE(search.mLines.empty());

    double score = search.mLines.front().score;
    ASSERT_GE(score, BestMove::WIN);

    int mateIn = test::matePlies(score, 5);
    EXPECT_EQ(mateIn, 5); // 3 plies from blue, 2 plies from red
}


TEST(Engine, Puzzle5_BlueWinIn4)
{
    // Load position from hash: Blue to move, wins in 4
    Node node = test::decodePosition("IAAAIAYBCAAAkD8u");

    BestMove search(7);
    node.accept(search);

    ASSERT_TRUE(search.mHasMove);
    ASSERT_FALSE(search.mLines.empty());

    double score = search.mLines.front().score;
    ASSERT_GE(score, BestMove::WIN);

    int mateIn = test::matePlies(score, 7);
    EXPECT_EQ(mateIn, 7); // 4 plies from blue, 3 plies from red
}


TEST(Engine, Puzzle6_BlueWinIn4)
{
    // Load position from hash: Blue to move, wins in 4
    Node node = test::decodePosition("A0CAAiAAAAAEGHZq");

    BestMove search(7);
    node.accept(search);

    ASSERT_TRUE(search.mHasMove);
    ASSERT_FALSE(search.mLines.empty());

    double score = search.mLines.front().score;
    ASSERT_GE(score, BestMove::WIN);

    int mateIn = test::matePlies(score, 7);
    EXPECT_EQ(mateIn, 7); // 4 plies from blue, 3 plies from red
}

