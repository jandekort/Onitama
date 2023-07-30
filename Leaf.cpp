//
// Created by Jan de Kort on 2019-08-18.
//

#include "Leaf.h"

void Leaf::addPiece(Piece a_piece) {
    m_piece->push_back(a_piece);
}

std::ostream& operator<<(std::ostream &out, const Leaf& lf) {

    char b0 = ' ';
    char b[5][5] = {{b0, b0, b0, b0, b0},
                    {b0, b0, b0, b0, b0},
                    {b0, b0, b0, b0, b0},
                    {b0, b0, b0, b0, b0},
                    {b0, b0, b0, b0, b0} };

    for(auto p: *lf.m_piece){
        b[p.mPosition.mX][p.mPosition.mY] = p.show();
    }

    for(int i=0; i<5; i++){
        for(int j=0; j<5; j++){
            out << '[' << b[i][j] << ']';
        }
        out << std::endl;
    }

    return out;
}
