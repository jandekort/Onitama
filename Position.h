//
// Created by Jan de Kort on 2019-08-18.
//

#ifndef ONITAMA_POSITION_H
#define ONITAMA_POSITION_H

struct Position
{
    Position() {};
    Position(int aX, int aY) : mX(aX), mY(aY) {};

    int mX;
    int mY;
    bool mIsValid;

    friend Position operator+(const Position& a, const Position& b)
    {
         Position r;
         /*r.m_x = a.m_x + b.m_x;
         r.m_y = a.m_y + b.m_y;*/
         return r;
    }
};

#endif //ONITAMA_POSITION_H
