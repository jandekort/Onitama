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
    return color == Piece::Red ? Card1_Red : Card1_Blue;
}

void Cards::play(int slot) {
    if (slot < Card1_Red || slot > Card2_Blue)
        throw std::logic_error("Cannot play that card slot");
    std::swap(mCards.at(slot), mCards.at(PlayedCard));
}

void Cards::show() const {
    std::cout << "Red:  " << CurrentCard::name(mCards.at(Card1_Red))
              << ", " << CurrentCard::name(mCards.at(Card2_Red)) << std::endl;
    std::cout << "Blue: " << CurrentCard::name(mCards.at(Card1_Blue))
              << ", " << CurrentCard::name(mCards.at(Card2_Blue)) << std::endl;
    std::cout << "Played card:  " << CurrentCard::name(mCards.at(PlayedCard)) << std::endl;
}
