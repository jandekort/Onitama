//
// Deck configuration: change CurrentCard to switch between deck implementations.
// OnitamaCards: 16 traditional Onitama animals
// ChessCards: Pawn, Bishop, Knight, Rook, King, Queen (chess-like)
//

#ifndef ONITAMA_DECKCONFIG_H
#define ONITAMA_DECKCONFIG_H

#include "OnitamaCards.h"
// Uncomment the line below to switch to ChessCards:
//#include "ChessCards.h"

using CurrentCard = OnitamaCards;
//using CurrentCard = ChessCards;

#endif //ONITAMA_DECKCONFIG_H
