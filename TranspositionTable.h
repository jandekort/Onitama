//
// A transposition table for the alpha-beta search: a fixed-size hash table
// keyed on a Zobrist hash of the game state. It lets the search reuse the
// result of a position no matter which move order reached it.
//
// Hashing is entirely the table's own business. It deliberately does NOT use
// StateEncoding (the UI's Base64 format): that class has its own job and must
// not change for the search's sake. The Zobrist scheme here is incremental-
// friendly, collision-resilient (64-bit keys are stored and checked), and
// independent of the textual encoding.
//

#ifndef ONITAMA_TRANSPOSITIONTABLE_H
#define ONITAMA_TRANSPOSITIONTABLE_H

#include "Ply.h"
#include <cstddef>
#include <cstdint>
#include <vector>

class Node;

// What a stored score tells us about the true value, relative to the window
// the position was searched with (standard alpha-beta bound flags).
enum class TTBound : std::uint8_t
{
    Exact, // value is the exact search value
    Lower, // value is a lower bound (a beta cut-off happened)
    Upper  // value is an upper bound (no move beat alpha)
};

struct TTEntry
{
    std::uint64_t key = 0;     // full Zobrist key, to detect index collisions
    double value = 0.0;        // mate-normalised score (see note in the .cpp)
    int depth = -1;            // remaining search depth this value was proven to
    TTBound bound = TTBound::Exact;
    Ply move;                  // best move found, for move ordering
    bool hasMove = false;
    bool valid = false;
};

class TranspositionTable
{
public:
    explicit TranspositionTable(std::size_t entries);

    // Compute the Zobrist hash of a state. Self-contained: depends only on the
    // pieces, the five card slots, and the side to move.
    std::uint64_t hash(const Node& node) const;

    // Look up a key. Returns true and fills `out` on a hit for that exact key.
    bool probe(std::uint64_t key, TTEntry& out) const;

    // Insert/replace. Depth-preferred: a shallower result never evicts a deeper
    // one for the same slot, so the most useful entries survive.
    void store(std::uint64_t key, int depth, double value, TTBound bound,
               const Ply& move, bool hasMove);

    void clear();

private:
    std::size_t index(std::uint64_t key) const { return key & mMask; }

    std::vector<TTEntry> mTable;
    std::size_t mMask; // table size is a power of two, so index = key & mMask
};

#endif // ONITAMA_TRANSPOSITIONTABLE_H
