//
// Created by Jan de Kort on 2019-08-20.
//

#include "Cards.h"

bool Cards::addCard(Animal aType) {
    int n = mCards.size();
    if(n < 5) {
        mCards.insert({n, std::make_shared<Animal>(aType) });
        return true;
    }
    else {
        return false;
    }
}

bool Cards::play(int aCard) {

    swap(aCard, 99);
    swap(CardExchange, aCard);
    swap(99, CardExchange);

    return true;
}

void Cards::show() {

    for(int i=0; i<5; i++)
    {
        auto it = mCards.find(i);
        std::cout << it->first << ": " << it->second->mType << std::endl;
    }
}

void Cards::swap(int oldKey, int newKey) {
    auto it = mCards.find(oldKey);
    if (it != mCards.end()) {
        std::swap(mCards[newKey], it->second);
        mCards.erase(it);
    }
    else
        throw std::logic_error("Key not found");
}

Move Cards::getMove(Cards::CardType aType, int aMove) {
    return mCards.at(aType)->getMove(aMove);
}

