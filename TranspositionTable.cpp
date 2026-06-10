#include "TranspositionTable.h"

#include "Node.h"
#include "Piece.h"
#include "DeckConfig.h"

#include <array>
#include <random>

namespace {

// Round up to the next power of two so the table size can be used as a bit
// mask. A power-of-two size means index() is a single AND instead of a modulo.
std::size_t roundUpPow2(std::size_t n)
{
    if (n < 1) return 1;
    std::size_t p = 1;
    while (p < n) p <<= 1;
    return p;
}

// Distinct piece kinds on a square: {Blue,Red} x {master,student}.
constexpr int kPieceKinds = 4;
constexpr int kSquares = 25;        // 5x5 board
constexpr int kCardSlots = 5;       // Card1_Red..Card2_Blue, PlayedCard
constexpr int kCardTypes = CurrentCard::CardCount;

// The Zobrist random table. Built once, deterministically, so every
// TranspositionTable (and every run) hashes a given state to the same key --
// important for the equivalence tests, which compare configs that do and do
// not use the table.
struct Zobrist
{
    std::array<std::array<std::uint64_t, kPieceKinds>, kSquares> piece;
    std::array<std::array<std::uint64_t, kCardTypes>, kCardSlots> card;
    std::uint64_t redToMove;

    Zobrist()
    {
        std::mt19937_64 rng(0x9E3779B97F4A7C15ull); // fixed seed -> stable keys
        std::uniform_int_distribution<std::uint64_t> dist;
        for (auto& sq : piece)
            for (auto& k : sq) k = dist(rng);
        for (auto& slot : card)
            for (auto& c : slot) c = dist(rng);
        redToMove = dist(rng);
    }
};

const Zobrist& zobrist()
{
    static const Zobrist z;
    return z;
}

int pieceKind(const Piece& p)
{
    int colorBit = (p.mColor == Piece::Red) ? 1 : 0;
    int masterBit = p.mIsMaster ? 1 : 0;
    return colorBit * 2 + masterBit; // 0..3
}

} // namespace

TranspositionTable::TranspositionTable(std::size_t entries)
    : mTable(roundUpPow2(entries)), mMask(mTable.size() - 1)
{
}

std::uint64_t TranspositionTable::hash(const Node& node) const
{
    const Zobrist& z = zobrist();
    std::uint64_t key = 0;

    for (const auto& p : node.m_piece) {
        int sq = p.mPosition.mY * 5 + p.mPosition.mX;
        key ^= z.piece[sq][pieceKind(p)];
    }

    for (int slot = 0; slot < kCardSlots; ++slot) {
        int type = static_cast<int>(node.m_cards.card(slot));
        key ^= z.card[slot][type];
    }

    if (node.m_tomove == Piece::Red)
        key ^= z.redToMove;

    return key;
}

bool TranspositionTable::probe(std::uint64_t key, TTEntry& out) const
{
    const TTEntry& e = mTable[index(key)];
    if (e.valid && e.key == key) {
        out = e;
        return true;
    }
    return false;
}

void TranspositionTable::store(std::uint64_t key, int depth, double value,
                               TTBound bound, const Ply& move, bool hasMove)
{
    TTEntry& slot = mTable[index(key)];

    // Depth-preferred replacement: keep the entry that was searched deeper,
    // except always refresh an entry for the same position (it is at least as
    // up to date). This makes the table favour the costliest-to-recompute
    // results without ever growing.
    if (slot.valid && slot.key != key && slot.depth > depth)
        return;

    slot.key = key;
    slot.value = value;
    slot.depth = depth;
    slot.bound = bound;
    slot.move = move;
    slot.hasMove = hasMove;
    slot.valid = true;
}

void TranspositionTable::clear()
{
    for (auto& e : mTable) e = TTEntry{};
}
