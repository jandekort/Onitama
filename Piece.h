//
// Created by Jan de Kort on 2019-08-18.
//

#ifndef ONITAMA_PIECE_H
#define ONITAMA_PIECE_H

#include "Position.h"
#include "PieceVisitor.h"

class PieceVisitor;

class Piece {

public:
    enum Color {Blue, Red};

    Piece(Position, Color = Blue, bool = false);

    void accept(PieceVisitor& p); // Do operation on Piece

    char show();

    Position mPosition;
    Color mColor;
    bool mIsMaster; // true->master, false->student

};

#endif //ONITAMA_PIECE_H
