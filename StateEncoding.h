#ifndef ONITAMA_STATE_ENCODING_H
#define ONITAMA_STATE_ENCODING_H

#include <string>

class Node;

namespace StateEncoding {

// Encode a game state to a compact Base64 hash string.
// Format: 96 bits packed as Base64 (16 characters)
//   - Board (75 bits): 3 bits per square (0=empty, 1=blue king, 2=blue pawn, 3=red king, 4=red pawn)
//   - Cards (20 bits): 4 bits per card slot × 5 slots
//   - Turn (1 bit): whose turn to move
std::string encodeState(const Node& node);

// Decode a Base64 hash string and populate the given Node with the state.
// Returns true on success, false if the hash is invalid.
bool decodeState(const std::string& encoded, Node& node);

} // namespace StateEncoding

#endif // ONITAMA_STATE_ENCODING_H
