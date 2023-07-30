//
// Created by Jan de Kort on 2019-08-20.
//

#include "Animal.h"

Animal::Animal(AnimalType aType)
    :
    mType(aType)
{
    switch(aType){
        case Dragon:
            mMove.push_back(Move( 2,  1));
            mMove.push_back(Move(-2,  1));
            mMove.push_back(Move( 1, -1));
            mMove.push_back(Move(-1, -1));
            break;
        case Rabbit:
            mMove.push_back(Move( 1,  1));
            mMove.push_back(Move( 2,  0));
            mMove.push_back(Move(-1, -1));
            break;
        case Tiger:
            mMove.push_back(Move( 0,  2));
            mMove.push_back(Move( 0, -1));
            break;
        case Elephant:
            mMove.push_back(Move( 1,  0));
            mMove.push_back(Move( 1,  1));
            mMove.push_back(Move(-1,  0));
            mMove.push_back(Move(-1,  1));
            break;
        case Cobra:
            mMove.push_back(Move(-1,  0));
            mMove.push_back(Move( 1,  1));
            mMove.push_back(Move( 1, -1));
            break;
        default:
            throw std::logic_error("Unknown animal");
    }
    mMoveCount = mMove.size();
}

Move Animal::getMove(int aMove) {
    show();
    return mMove.at(aMove);
}

void Animal::show() {
    for(auto m : mMove) {
        std::cout << "mShiftX = " << m.mShiftX << " mShiftY = " << m.mShiftY << std::endl;
    }
}

