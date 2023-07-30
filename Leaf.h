//
// Created by Jan de Kort on 2019-08-18.
//

#ifndef ONITAMA_LEAF_H
#define ONITAMA_LEAF_H

#include <vector>
#include <memory>
#include <iostream>
#include <string>

#include "Piece.h"

class Leaf {
public:
    Leaf() : m_piece(std::make_shared<std::vector<Piece> >() ) {};

    void addPiece(Piece a_piece);

    friend std::ostream& operator << (std::ostream& out, const Leaf& lf);

    std::shared_ptr<std::vector<Piece> > m_piece;
    Piece::Color m_tomove;

};


#endif //ONITAMA_LEAF_H
