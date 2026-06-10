//
// Created by Jan de Kort on 2019-08-18.
//

#ifndef ONITAMA_PIECEVISITOR_H
#define ONITAMA_PIECEVISITOR_H

#include "Piece.h"
#include <iostream>

class Piece;

class PieceVisitor // Base class of operations on Piece
{
public:
    virtual void visit(Piece& p) = 0;
    virtual ~PieceVisitor() {};
};

class Move : public PieceVisitor
{
public:
    Move(int, int);
    void visit(Piece& p);

//private:
    int mShiftX, mShiftY;
};


#endif //ONITAMA_PIECEVISITOR_H
