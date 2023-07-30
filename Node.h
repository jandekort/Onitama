//
// Created by Jan de Kort on 2019-08-18.
//

#ifndef ONITAMA_Node_H
#define ONITAMA_Node_H

#include "NodeVisitor.h"
#include <vector>
#include <memory>
#include <iostream>
#include <string>

#include "Piece.h"

class NodeVisitor;

class Node {
public:
    Node() : m_piece(std::make_shared<std::vector<Piece> >() ) {};

    void addPiece(Piece a_piece);

    void accept(NodeVisitor& n); // Do operation on Node

    friend std::ostream& operator << (std::ostream& out, const Node& lf);

    std::shared_ptr<std::vector<Piece> > m_piece;
    Piece::Color m_tomove;
    double m_score;

};


#endif //ONITAMA_Node_H
