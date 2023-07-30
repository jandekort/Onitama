//
// Created by Jan de Kort on 2019-08-18.
//

#include "NodeVisitor.h"

void Score::visit(Node &n)
{
    double score = 0.0;
    for(auto p: *n.m_piece)
    {
        int x = p.mPosition.mX;
        int y = p.mPosition.mY;
        score += abs(x - 2) + abs(y - 2);
    }
    n.m_score = score;
}

