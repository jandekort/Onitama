//
// Created by Jan de Kort on 2019-08-20.
//

#ifndef ONITAMA_CARDS_H
#define ONITAMA_CARDS_H

#include <map>
#include "PieceVisitor.h"
#include "Animal.h"

class Cards{
public:

    enum CardType {CardA1 = 0, CardA2, CardB1, CardB2, CardExchange};

    Cards() {};

    bool addCard(Animal);
    bool play(int);
    void show();
    Move getMove(CardType, int);

private:
    void swap(int, int);

    std::map<int, std::shared_ptr<Animal>> mCards;
};

#endif //ONITAMA_CARDS_H
