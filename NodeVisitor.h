//
// Created by Jan de Kort on 2019-08-18.
//

#ifndef ONITAMA_NODEVISITOR_H
#define ONITAMA_NODEVISITOR_H

#include "Ply.h"
#include "SearchConfig.h"
#include <chrono>
#include <iostream>
#include <memory>
#include <vector>

class Node;
class TranspositionTable;

// A line found by the search: its principal variation (the sequence of plies
// starting from the searched position) and the score it leads to, from that
// position's side-to-move perspective. Scores use the engine's WIN+depth
// mate encoding, so |score| >= WIN means a forced win/loss.
struct ScoredLine
{
    std::vector<Ply> line;
    double score = 0.0;
};

class NodeVisitor // Base class of operations on Node
{
public:
    virtual void visit(Node& n) = 0;
    virtual ~NodeVisitor() {};
};

// Static evaluation of a Node, from the perspective of the side to move.
// Positive = good for the side to move. Stored in Node::m_score.
class Score : public NodeVisitor
{
public:
    Score() {};
    void visit(Node& n) override;

};

// Looks a number of plies ahead (negamax with alpha-beta pruning) and
// stores the best ply found in mBest. Also sets Node::m_score to the
// search value from the side to move's perspective.
//
// aMultiPV sets how many top lines (sorted best-first) it keeps in mLines.
// The search always uses ordinary alpha-beta, so it costs the same regardless
// of aMultiPV; only the best line's score is exact, the others carry the
// bounded value the pruned search reached.
//
// aConfig opts into search optimizations (transposition table, iterative
// deepening). They are additive: the default config reproduces the plain
// alpha-beta search exactly, and every enabled combination returns the same
// best move and the same best-line score on a given position -- only the work
// done (mNodes) changes. See SearchConfig and TranspositionTable.
class BestMove : public NodeVisitor
{
public:
    static const double WIN; // value of a won position

    explicit BestMove(int aDepth, int aMultiPV = 1,
                      SearchConfig aConfig = SearchConfig{});
    ~BestMove() override;

    void visit(Node& n) override;

    Ply mBest;
    bool mHasMove;
    long mNodes;                     // positions examined, for curiosity
    std::vector<ScoredLine> mLines;  // best lines found, sorted best-first

    // Iterative-deepening / time-budget feedback. mDepthReached is the deepest
    // fully completed depth (== the requested depth unless cut short); mTimedOut
    // is true when the time budget stopped the search before that depth.
    int mDepthReached;
    bool mTimedOut;

    // Root moves to skip, matched by value. Lets a caller enumerate alternative
    // best lines: search, note the best move, add it here, search again. Empty
    // by default, so ordinary searches are unaffected.
    std::vector<Ply> mExcludeRoot;

private:
    // One fixed-depth root search. Fills mLines/mBest/mHasMove on completion;
    // if the time budget aborts it mid-pass it commits nothing, so the previous
    // completed depth's result survives. Iterative deepening just calls this for
    // increasing depths, reusing the table.
    void searchRoot(Node& n, int depth);

    double negamax(const Node& n, int depth, double alpha, double beta,
                   std::vector<Ply>& pv);

    int mDepth;
    int mMultiPV;
    SearchConfig mConfig;
    std::unique_ptr<TranspositionTable> mTT; // null unless enabled by mConfig

    // Time-budget bookkeeping for the current search.
    std::chrono::steady_clock::time_point mDeadline;
    bool mUseDeadline; // whether mDeadline is enforced this pass
    bool mAborted;     // set when the deadline is hit; unwinds the search
};

#endif //ONITAMA_NODEVISITOR_H
