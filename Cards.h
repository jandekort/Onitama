//
// Created by Jan de Kort on 2019-08-20.
//

#ifndef ONITAMA_CARDS_H
#define ONITAMA_CARDS_H

#include "DeckConfig.h"
#include <array>
#include <random>

// The five cards in play. A cheap value type: it is copied into every
// search Node, because card rotation is part of the game state.
class Cards{
public:

    // A = Red (top player), B = Blue (bottom player)
    enum CardType {CardA1 = 0, CardA2, CardB1, CardB2, CardExchange};

    Cards();

    void deal(std::mt19937& rng);          // draw 5 distinct cards from the deck

    CurrentCard::CardType card(int slot) const { return mCards.at(slot); }

    void set(int slot, CurrentCard::CardType type) { mCards.at(slot) = type; }

    static int firstSlot(int color);        // Piece::Color -> first hand slot

    void play(int slot);                    // swap played card with side card
    void show() const;

private:
    std::array<CurrentCard::CardType, 5> mCards;
};

#endif //ONITAMA_CARDS_H
