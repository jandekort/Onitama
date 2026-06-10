#include <random>
#include <sstream>
#include <string>

#include "Node.h"
#include "NodeVisitor.h"
#include "DeckConfig.h"
#include "Cards.h"
#include "PuzzleGenerator.h"
#include "StateEncoding.h"

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#else
#define EMSCRIPTEN_KEEPALIVE
#endif

extern "C" {

// Find a win-in-N puzzle. Returns JSON: {"found":true/false, "pieces":[...], "cards":{...}}
// timeoutSeconds bounds the search so deep puzzles (high N) can be given more
// time from the UI instead of the previous fixed budget.
EMSCRIPTEN_KEEPALIVE
const char* puzzle_find(int depth, int maxPawns, unsigned seed, int timeoutSeconds) {
    static std::string result;

    PuzzleGenerator::Puzzle puzzle =
        PuzzleGenerator::findPuzzle(depth, maxPawns, seed, timeoutSeconds);

    std::ostringstream out;
    if (!puzzle.found) {
        out << "{\"found\":false}";
        result = out.str();
        return result.c_str();
    }

    out << "{\"found\":true,";

    out << "\"pieces\":[";
    bool first = true;
    for (const auto& p : puzzle.node.m_piece) {
        if (!first) out << ",";
        first = false;
        out << "{\"x\":" << p.mPosition.mX << ",\"y\":" << p.mPosition.mY
            << ",\"color\":\"" << (p.mColor == Piece::Red ? "red" : "blue")
            << "\",\"master\":" << (p.mIsMaster ? "true" : "false") << "}";
    }
    out << "],";

    // Helper lambda for card serialization
    auto serializeCard = [](const Cards& cards, int slot) -> std::string {
        std::ostringstream s;
        const auto card = cards.card(slot);
        s << "{\"name\":\"" << CurrentCard::name(card) << "\",\"moves\":[";
        bool firstMove = true;
        for (const auto& m : CurrentCard::movesFor(card)) {
            if (!firstMove) s << ",";
            firstMove = false;
            s << "[" << m.mShiftX << "," << m.mShiftY << "]";
        }
        s << "]}";
        return s.str();
    };

    out << "\"cards\":{";
    out << "\"red\":[" << serializeCard(puzzle.node.m_cards, Cards::Card1_Red) << ","
        << serializeCard(puzzle.node.m_cards, Cards::Card2_Red) << "],";
    out << "\"blue\":[" << serializeCard(puzzle.node.m_cards, Cards::Card1_Blue) << ","
        << serializeCard(puzzle.node.m_cards, Cards::Card2_Blue) << "],";
    out << "\"played\":" << serializeCard(puzzle.node.m_cards, Cards::PlayedCard);
    out << "},";

    // Encode the puzzle position as a state hash
    std::string encoded = StateEncoding::encodeState(puzzle.node);
    out << "\"hash\":\"" << encoded << "\",";

    // Debug info: verify the puzzle by searching to find the mate depth.
    // TT + iterative deepening on (correctness-neutral, fewer nodes).
    BestMove debugSearch(2 * depth, 1, SearchConfig::all());
    Node debugNode = puzzle.node;
    debugNode.accept(debugSearch);
    int debugMatePlies = 0;
    if (debugSearch.mLines.size() > 0 && debugSearch.mLines[0].score >= BestMove::WIN) {
        debugMatePlies = 2 * depth - (debugSearch.mLines[0].score - BestMove::WIN);
    }

    out << "\"debug\":{\"pieces\":" << puzzle.node.m_piece.size()
        << ",\"verified_mate_plies\":" << debugMatePlies
        << ",\"target_mate_plies\":" << (2 * depth - 1) << "}}";

    result = out.str();
    return result.c_str();
}

} // extern "C"
