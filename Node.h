//
// Created by Jan de Kort on 2019-08-18.
//

#ifndef ONITAMA_Node_H
#define ONITAMA_Node_H

#include <vector>
#include <iostream>
#include <string>

#include "Piece.h"
#include "Cards.h"
#include "Ply.h"

class NodeVisitor;

// A node in the search tree: a complete game state.
// Plain value semantics so the search can copy it freely.
class Node {
public:
    Node() : m_tomove(Piece::Blue), m_score(0.0) {};

    void addPiece(Piece a_piece);

    void accept(NodeVisitor& n); // Do operation on Node

    const Piece* pieceAt(Position aPos) const;
    Piece* pieceAt(Position aPos);

    // The square where a color's master starts: its Temple Arch.
    static Position temple(Piece::Color aColor);

    // True if the side to move has already lost: its master was captured
    // (Way of the Stone) or the enemy master sits on its temple arch
    // (Way of the Stream).
    bool isLoss() const;

    // All legal plies for the side to move. Never empty: if no piece can
    // move, the player must still exchange a card (pass plies).
    std::vector<Ply> legalMoves() const;

    // The state after playing a ply: piece moved, capture resolved,
    // card rotated, side to move switched.
    Node applyPly(const Ply& aPly) const;

    friend std::ostream& operator << (std::ostream& out, const Node& lf);

    std::vector<Piece> m_piece;
    Cards m_cards;
    Piece::Color m_tomove;
    double m_score;

};


#endif //ONITAMA_Node_H
