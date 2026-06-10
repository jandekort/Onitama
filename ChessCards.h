//
// Chess-like card deck: Pawn, Bishop, Knight, Rook, King, Queen.
//

#ifndef ONITAMA_CHESSCARDS_H
#define ONITAMA_CHESSCARDS_H

#include "PieceVisitor.h"
#include <vector>
#include <array>
#include <random>

struct ChessCards {

    // Chess-like cards
    enum CardType {
        Pawn = 0,      // Pawn: moves forward one square
        Bishop = 1,    // Bishop: moves diagonally
        Knight = 2,    // Knight: L-shape moves
        Rook = 3,      // Rook: moves horizontally/vertically
        King = 4,      // King: moves one square in any direction
        Queen = 5,     // Queen: moves to the four corners
        CardCount = 6
    };

    ChessCards() : mType(King) {};
    ChessCards(CardType);

    // Offsets as seen by the owner: Blue (bottom) applies them as-is,
    // Red (top) negates both axes (the 180-degree card rotation).
    // Forward = -y on the board array (towards the top of the printed board).
    static const std::vector<Move>& movesFor(CardType);

    static const char* name(CardType);

    static std::array<CardType, 5> getInitialHand(std::mt19937& rng);

    void show() const; // print the card's move diagram (owner's perspective)

    CardType mType;

};

#endif //ONITAMA_CHESSCARDS_H
