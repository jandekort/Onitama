//
// Created by Jan de Kort on 2019-08-20.
//

#ifndef ONITAMA_ONITAMACARDS_H
#define ONITAMA_ONITAMACARDS_H

#include "PieceVisitor.h"
#include <vector>
#include <array>
#include <random>

struct OnitamaCards {

    // All 16 cards from Rules.txt
    enum CardType {
        Tiger = 0, Dragon, Frog, Rabbit,
        Crab, Elephant, Goose, Rooster,
        Monkey, Mantis, Horse, Ox,
        Crane, Boar, Eel, Cobra,
        CardCount
    };

    OnitamaCards() : mType(Tiger) {};
    OnitamaCards(CardType);

    // Offsets as seen by the owner: Blue (bottom) applies them as-is,
    // Red (top) negates both axes (the 180-degree card rotation).
    // Forward = -y on the board array (towards the top of the printed board).
    static const std::vector<Move>& movesFor(CardType);

    static const char* name(CardType);

    static std::array<CardType, 5> getInitialHand(std::mt19937& rng);

    void show() const; // print the card's move diagram (owner's perspective)

    CardType mType;

};

#endif //ONITAMA_ONITAMACARDS_H
