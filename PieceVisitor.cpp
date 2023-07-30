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
    int x = p.mPosition.mX + mShiftX;
    int y = p.mPosition.mY + mShiftY;

    if( x < 5 && x >= 0 && y < 5 && y >= 0 )
        p.mPosition = Position(x, y);
    else
        std::cout << "Invalid move: " << x << ", " << y << std::endl;
}
