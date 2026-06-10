//
// Created by Jan de Kort on 2019-08-18.
//

#include "PieceVisitor.h"
#include "Position.h"
#include "Piece.h"

Move::Move(int aShiftX, int aShiftY)
    :
    mShiftX(aShiftX),
    mShiftY(aShiftY)
{
}

void Move::visit(Piece &p)
{
    // Validity (board bounds, collisions) is checked by Node::legalMoves();
    // by the time a Move is applied it is known to be legal.
    p.mPosition = p.mPosition + Position(mShiftX, mShiftY);
}
