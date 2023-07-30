//
// Created by Jan de Kort on 2019-08-20.
//

#ifndef ONITAMA_ANIMAL_H
#define ONITAMA_ANIMAL_H

#include "PieceVisitor.h"
#include <vector>

struct Animal {

    enum AnimalType {Dragon, Rabbit, Tiger, Elephant, Cobra};

    Animal() {};
    Animal(AnimalType);

    Move getMove(int);

    void show();

    std::vector<Move> mMove;
    AnimalType mType;
    int mMoveCount;

};

#endif //ONITAMA_ANIMAL_H
