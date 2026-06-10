#ifndef ONITAMA_TEST_HELPERS_H
#define ONITAMA_TEST_HELPERS_H

#include <cmath>
#include <iostream>
#include <string>

#include "Node.h"
#include "NodeVisitor.h"
#include "Cards.h"
#include "DeckConfig.h"
#include "StateEncoding.h"

namespace test
{

// Convert chess-style square names to board position
inline Position sq(const std::string& name)
{
    int x = name[0] - 'a';
    int rank = name[1] - '0';
    return Position(x, 5 - rank);
}

// Builder for a board position (cards default to all-Tiger)
class Board
{
public:
    Board& toMove(Piece::Color c) { mNode.m_tomove = c; return *this; }

    Board& master(Piece::Color c, const std::string& at) {
        mNode.addPiece(Piece(sq(at), c, true));
        return *this;
    }
    Board& student(Piece::Color c, const std::string& at) {
        mNode.addPiece(Piece(sq(at), c, false));
        return *this;
    }

    // Set all five card slots
    Board& cards(CurrentCard::CardType card1Red, CurrentCard::CardType card2Red,
                 CurrentCard::CardType card1Blue, CurrentCard::CardType card2Blue,
                 CurrentCard::CardType playedCard) {
        mNode.m_cards.set(Cards::Card1_Red, card1Red);
        mNode.m_cards.set(Cards::Card2_Red, card2Red);
        mNode.m_cards.set(Cards::Card1_Blue, card1Blue);
        mNode.m_cards.set(Cards::Card2_Blue, card2Blue);
        mNode.m_cards.set(Cards::PlayedCard, playedCard);
        return *this;
    }

    Node build() const { return mNode; }

private:
    Node mNode;
};

// When the engine finds a forced mate, it returns score = WIN + offset
// The offset is designed so the engine prefers faster mates
// Mate in 2 at depth 8:  score = WIN + 6  (high score)
// Mate in 4 at depth 8:  score = WIN + 4  (lower score)
// Mate in 8 at depth 8:  score = WIN + 0  (lowest)
inline int matePlies(double score, int depth)
{
    return depth - static_cast<int>(std::llround(score - BestMove::WIN));
}

// Encode a position as a state hash. Uncomment the std::cout line to output hashes for new test positions.
inline std::string encodePosition(const Node& node)
{
    std::string hash = StateEncoding::encodeState(node);
    // std::cout << "Position hash: " << hash << std::endl;
    return hash;
}

// Load a position from a state hash string.
inline Node decodePosition(const std::string& hash)
{
    Node node;
    StateEncoding::decodeState(hash, node);
    return node;
}

} // namespace test

#endif // ONITAMA_TEST_HELPERS_H
