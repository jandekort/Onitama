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
        // Tiger: 2 forward, 1 backward
        sMoves[Tiger]    = { Move( 0, -2), Move( 0,  1) };
        // Dragon: long forward diagonals, short backward diagonals
        sMoves[Dragon]   = { Move(-2, -2), Move( 2, -2), Move(-1,  1), Move( 1,  1) };
        // Frog: left, forward-left, forward-right
        sMoves[Frog]     = { Move(-1,  0), Move(-1, -1), Move( 1, -1) };
        // Rabbit: right, forward-right, forward-left
        sMoves[Rabbit]   = { Move( 1,  0), Move( 1, -1), Move(-1, -1) };
        // Crab: forward, left, right
        sMoves[Crab]     = { Move( 0, -1), Move(-1,  0), Move( 1,  0) };
        // Elephant: forward-left, forward-right, left, right
        sMoves[Elephant] = { Move(-1, -1), Move( 1, -1), Move(-1,  0), Move( 1,  0) };
        // Goose: forward-left, left, right, backward-right
        sMoves[Goose]    = { Move(-1, -1), Move(-1,  0), Move( 1,  0), Move( 1,  1) };
        // Rooster: forward-right, right, left, backward-left
        sMoves[Rooster]  = { Move( 1, -1), Move( 1,  0), Move(-1,  0), Move(-1,  1) };
        // Monkey: any diagonal adjacent square
        sMoves[Monkey]   = { Move(-1, -1), Move( 1, -1), Move(-1,  1), Move( 1,  1) };
        // Mantis: forward-left, forward-right, backward
        sMoves[Mantis]   = { Move(-1, -1), Move( 1, -1), Move( 0,  1) };
        // Horse: forward, left, backward
        sMoves[Horse]    = { Move( 0, -1), Move(-1,  0), Move( 0,  1) };
        // Ox: forward, right, backward
        sMoves[Ox]       = { Move( 0, -1), Move( 1,  0), Move( 0,  1) };
        // Crane: forward, backward-left, backward-right
        sMoves[Crane]    = { Move( 0, -1), Move(-1,  1), Move( 1,  1) };
        // Boar: forward, left, right
        sMoves[Boar]     = { Move( 0, -1), Move(-1,  0), Move( 1,  0) };
        // Eel: forward-left, right, backward-left
        sMoves[Eel]      = { Move(-1, -1), Move( 1,  0), Move(-1,  1) };
        // Cobra: forward-right, left, backward-right
        sMoves[Cobra]    = { Move( 1, -1), Move(-1,  0), Move( 1,  1) };
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
