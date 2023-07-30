//
// Created by Jan de Kort on 2019-08-18.
//

#ifndef ONITAMA_NODEVISITOR_H
#define ONITAMA_NODEVISITOR_H

#include "Node.h"
#include <iostream>

class Node;

class NodeVisitor // Base class of operations on Node
{
public:
    virtual void visit(Node& n) = 0;
};

class Score : public NodeVisitor
{
public:
    Score() {};
    void visit(Node& n) override;

};

#endif //ONITAMA_NODEVISITOR_H
