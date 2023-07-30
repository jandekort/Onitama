//
// Created by Jan de Kort on 2019-08-18.
//

#include "Piece.h"

Piece::Piece(Position aPosition,
        Color aColor /* =Red */,
        bool aIsMaster /* =false */ )
    :
    mPosition(aPosition),
    mColor(aColor),
    mIsMaster(aIsMaster)
{
}

char Piece::show() {
    switch(mColor)
    {
        case Red:
            if(mIsMaster) return 'R'; else return 'r';
            break;
        case Blue:
            if(mIsMaster) return 'B'; else return 'b';
            break;
    }
}

void Piece::accept(PieceVisitor &p)
{
    p.visit(*this);
}

