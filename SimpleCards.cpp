//
// Simple chess-like card deck: 4 Kings and 1 Knight.
//

#include "SimpleCards.h"
#include <stdexcept>
#include <algorithm>

const std::vector<Move>& SimpleCards::movesFor(CardType aType) {

    static std::vector<Move> sMoves[CardCount];
    static bool sInitialized = false;

    if (!sInitialized) {
        // Pawn: moves forward one square
        sMoves[Pawn] = {
            Move( 0, -1)   // forward
        };

        // Bishop: moves diagonally
        sMoves[Bishop] = {
            Move(-1, -1),  // forward-left
            Move( 1, -1),  // forward-right
            Move(-1,  1),  // backward-left
            Move( 1,  1)   // backward-right
        };

        // Knight: L-shape (2 squares one direction, 1 square perpendicular)
        sMoves[Knight] = {
            Move( 2, -1),  // right-forward
            Move( 2,  1),  // right-backward
            Move(-2, -1),  // left-forward
            Move(-2,  1),  // left-backward
            Move( 1, -2),  // forward-right
            Move(-1, -2),  // forward-left
            Move( 1,  2),  // backward-right
            Move(-1,  2)   // backward-left
        };

        // Rook: moves horizontally and vertically
        sMoves[Rook] = {
            Move( 0, -1),  // forward
            Move( 0,  1),  // backward
            Move(-1,  0),  // left
            Move( 1,  0)   // right
        };

        // King: one square in any direction
        sMoves[King] = {
            Move( 0, -1),  // forward
            Move( 0,  1),  // backward
            Move(-1,  0),  // left
            Move( 1,  0),  // right
            Move(-1, -1),  // forward-left
            Move( 1, -1),  // forward-right
            Move(-1,  1),  // backward-left
            Move( 1,  1)   // backward-right
        };

        // Queen: moves to the four corners (2 squares diagonally)
        sMoves[Queen] = {
            Move(-2, -2),  // forward-left
            Move( 2, -2),  // forward-right
            Move(-2,  2),  // backward-left
            Move( 2,  2)   // backward-right
        };

        sInitialized = true;
    }

    if (aType < 0 || aType >= CardCount)
        throw std::logic_error("Unknown card");

    return sMoves[aType];
}

const char* SimpleCards::name(CardType aType) {
    static const char* sNames[CardCount] = {
        "Pawn", "Bishop", "Knight", "Rook", "King", "Queen"
    };
    if (aType < 0 || aType >= CardCount)
        throw std::logic_error("Unknown card");
    return sNames[aType];
}

SimpleCards::SimpleCards(CardType aType)
    : mType(aType)
{
}

void SimpleCards::show() const {
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

std::array<SimpleCards::CardType, 5> SimpleCards::getInitialHand(std::mt19937& rng) {
    // Deal 5 random distinct cards from the 6 available (rng selects which card to exclude)
    std::array<CardType, 6> allCards = {Pawn, Bishop, Knight, Rook, King, Queen};
    std::shuffle(allCards.begin(), allCards.end(), rng);

    std::array<CardType, 5> hand = {
        allCards[0], allCards[1], allCards[2], allCards[3], allCards[4]
    };
    return hand;
}
