//
// One legal move (a "ply"): which card is played and which piece goes where.
// If no piece can move, the player must still exchange a card: mIsPass = true.
//

#ifndef ONITAMA_PLY_H
#define ONITAMA_PLY_H

#include "Position.h"

struct Ply
{
    Ply() : mCardSlot(-1), mIsPass(true) {};

    int mCardSlot;   // Cards slot (Card1_Red..Card2_Blue) being played
    bool mIsPass;    // true -> only the card is exchanged, no piece moves
    Position mFrom;
    Position mTo;
};

#endif //ONITAMA_PLY_H
