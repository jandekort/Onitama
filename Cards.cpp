//
// Created by Jan de Kort on 2019-08-20.
//

#include "Cards.h"
#include "Piece.h"
#include <iostream>

Cards::Cards() {
    mCards.fill(static_cast<CurrentCard::CardType>(0));  // First card in the deck
}

void Cards::deal(std::mt19937& rng) {
    auto hand = CurrentCard::getInitialHand(rng);
    for (int i = 0; i < 5; i++)
        mCards.at(i) = hand.at(i);
}

int Cards::firstSlot(int color) {
    return color == Piece::Red ? CardA1 : CardB1;
}

void Cards::play(int slot) {
    if (slot < CardA1 || slot > CardB2)
        throw std::logic_error("Cannot play that card slot");
    std::swap(mCards.at(slot), mCards.at(CardExchange));
}

void Cards::show() const {
    std::cout << "Red:  " << CurrentCard::name(mCards.at(CardA1))
              << ", " << CurrentCard::name(mCards.at(CardA2)) << std::endl;
    std::cout << "Blue: " << CurrentCard::name(mCards.at(CardB1))
              << ", " << CurrentCard::name(mCards.at(CardB2)) << std::endl;
    std::cout << "Side card:  " << CurrentCard::name(mCards.at(CardExchange)) << std::endl;
}
