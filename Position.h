//
// Created by Jan de Kort on 2019-08-18.
//

#ifndef ONITAMA_POSITION_H
#define ONITAMA_POSITION_H

struct Position
{
    Position() : mX(0), mY(0) {};
    Position(int aX, int aY) : mX(aX), mY(aY) {};

    int mX;
    int mY;

    bool isOnBoard() const
    {
        return mX >= 0 && mX < 5 && mY >= 0 && mY < 5;
    }

    friend Position operator+(const Position& a, const Position& b)
    {
        return Position(a.mX + b.mX, a.mY + b.mY);
    }

    friend bool operator==(const Position& a, const Position& b)
    {
        return a.mX == b.mX && a.mY == b.mY;
    }

    friend bool operator!=(const Position& a, const Position& b)
    {
        return !(a == b);
    }
};

#endif //ONITAMA_POSITION_H
