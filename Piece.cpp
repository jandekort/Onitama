//
// Created by Jan de Kort on 2019-08-18.
//

#include "Piece.h"
#include "PieceVisitor.h"

Piece::Piece(Position aPosition,
        Color aColor /* =Blue */,
        bool aIsMaster /* =false */ )
    :
    mPosition(aPosition),
    mColor(aColor),
    mIsMaster(aIsMaster)
{
}

char Piece::show() const {
    switch(mColor)
    {
        case Red:
            return mIsMaster ? 'R' : 'r';
        case Blue:
            return mIsMaster ? 'B' : 'b';
    }
    return '?';
}

void Piece::accept(PieceVisitor &p)
{
    p.visit(*this);
}
