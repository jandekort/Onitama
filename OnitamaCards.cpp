//
// Created by Jan de Kort on 2019-08-20.
//

#include "OnitamaCards.h"
#include <stdexcept>
#include <algorithm>
#include <numeric>

const std::vector<Move>& OnitamaCards::movesFor(CardType aType) {

    static std::vector<Move> sMoves[CardCount];
    static bool sInitialized = false;

    if (!sInitialized) {
        sMoves[Tiger]    = { Move( 0, -2), Move( 0,  1) };
        sMoves[Dragon]   = { Move(-2, -2), Move( 2, -2), Move(-1,  1), Move( 1,  1) };
        sMoves[Frog]   = { Move(-1, -1), Move(-2,  0), Move( 1,  1) };
        sMoves[Rabbit] = { Move( 1, -1), Move( 2,  0), Move(-1,  1) };
        sMoves[Crab]   = { Move( 0, -1), Move(-2,  0), Move( 2,  0) };
        sMoves[Elephant] = { Move(-1, -1), Move( 1, -1), Move(-1,  0), Move( 1,  0) };
        sMoves[Goose]    = { Move(-1, -1), Move(-1,  0), Move( 1,  0), Move( 1,  1) };
        sMoves[Rooster]  = { Move( 1, -1), Move( 1,  0), Move(-1,  0), Move(-1,  1) };
        sMoves[Monkey]   = { Move(-1, -1), Move( 1, -1), Move(-1,  1), Move( 1,  1) };
        sMoves[Mantis]   = { Move(-1, -1), Move( 1, -1), Move( 0,  1) };
        sMoves[Horse]    = { Move( 0, -1), Move(-1,  0), Move( 0,  1) };
        sMoves[Crane]    = { Move( 0, -1), Move(-1,  1), Move( 1,  1) };
        sMoves[Boar]     = { Move( 0, -1), Move(-1,  0), Move( 1,  0) };
        sMoves[Eel]      = { Move(-1, -1), Move( 1,  0), Move(-1,  1) };
        sMoves[Cobra]    = { Move( 1, -1), Move(-1,  0), Move( 1,  1) };
        sMoves[Ox]       = { Move( 0, -1), Move( 1,  0), Move( 0,  1) };
        sInitialized = true;
    }

    if (aType < 0 || aType >= CardCount)
        throw std::logic_error("Unknown card");

    return sMoves[aType];
}

const char* OnitamaCards::name(CardType aType) {
    static const char* sNames[CardCount] = {
        "Tiger", "Dragon", "Frog", "Rabbit",
        "Crab", "Elephant", "Goose", "Rooster",
        "Monkey", "Mantis", "Horse", "Ox",
        "Crane", "Boar", "Eel", "Cobra"
    };
    if (aType < 0 || aType >= CardCount)
        throw std::logic_error("Unknown card");
    return sNames[aType];
}

OnitamaCards::OnitamaCards(CardType aType)
    : mType(aType)
{
}

void OnitamaCards::show() const {
    // 5x5 diagram, owner's perspective: O in the centre, X = destination.
    const auto& moves = movesFor(mType);
    for (int dy = -2; dy <= 2; dy++) {
        std::cout << "    ";
        for (int dx = -2; dx <= 2; dx++) {
            char c = '.';
            if (dx == 0 && dy == 0)
                c = 'O';
            else
                for (const auto& m : moves)
                    if (m.mShiftX == dx && m.mShiftY == dy) c = 'X';
            std::cout << c << ' ';
        }
        std::cout << std::endl;
    }
}

std::array<OnitamaCards::CardType, 5> OnitamaCards::getInitialHand(std::mt19937& rng) {
    std::array<int, CardCount> deck;
    std::iota(deck.begin(), deck.end(), 0);
    std::shuffle(deck.begin(), deck.end(), rng);

    std::array<CardType, 5> hand;
    for (int i = 0; i < 5; i++)
        hand[i] = static_cast<CardType>(deck[i]);
    return hand;
}
