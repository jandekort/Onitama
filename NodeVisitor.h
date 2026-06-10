//
// Created by Jan de Kort on 2019-08-18.
//

#ifndef ONITAMA_NODEVISITOR_H
#define ONITAMA_NODEVISITOR_H

#include "Ply.h"
#include <iostream>
#include <vector>

class Node;

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
class BestMove : public NodeVisitor
{
public:
    static const double WIN; // value of a won position

    explicit BestMove(int aDepth, int aMultiPV = 1)
        : mHasMove(false), mNodes(0), mDepth(aDepth), mMultiPV(aMultiPV) {};
    void visit(Node& n) override;

    Ply mBest;
    bool mHasMove;
    long mNodes;                     // positions examined, for curiosity
    std::vector<ScoredLine> mLines;  // best lines found, sorted best-first

private:
    double negamax(const Node& n, int depth, double alpha, double beta,
                   std::vector<Ply>& pv);

    int mDepth;
    int mMultiPV;
};

#endif //ONITAMA_NODEVISITOR_H
