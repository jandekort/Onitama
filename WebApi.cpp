//
// WebAssembly bridge: exposes the Onitama engine to JavaScript.
// Compiled with Emscripten instead of main.cpp; the JS frontend calls
// these functions via cwrap and receives the whole game state as JSON.
//

#include <random>
#include <sstream>
#include <string>
#include <vector>

#include "Node.h"
#include "NodeVisitor.h"
#include "DeckConfig.h"
#include "Cards.h"
#include "StateEncoding.h"

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#else
#define EMSCRIPTEN_KEEPALIVE
#endif

namespace {

Node gNode;                 // current game state
std::vector<Ply> gLegal;    // legal plies for the side to move
Ply gLastPly;               // last ply played (for highlighting)
bool gHasLastPly = false;
std::string gJson;          // buffer returned to JS

// The engine's analysis of the current position: best line(s) found, sorted
// best-first, from gNode's side-to-move perspective. Populated by a search
// (og_eval) or as a byproduct of the engine's own move (og_ai_move), and
// invalidated whenever the position changes by other means.
std::vector<ScoredLine> gAnalysis;
bool gHasAnalysis = false;

// The engine's top candidate lines from the move it last played, already
// serialized to JSON (from the pre-move position, so cards/sides are correct).
// This is exactly the search behind the current score bar, so its first line
// matches the bar. Empty until the engine has moved.
std::string gEngineLinesJson;
bool gHasEngineLines = false;

// Snapshot of the state before each ply, so plies can be taken back. The
// analysis and engine lines are stored too, so taking back restores the
// position's cached score/lines instead of forcing a fresh re-search.
struct HistoryEntry {
    Node node;
    Ply lastPly;
    bool hasLastPly;
    std::vector<ScoredLine> analysis;
    bool hasAnalysis;
    std::string engineLines;
    bool hasEngineLines;
};
std::vector<HistoryEntry> gHistory;

void pushHistory() {
    gHistory.push_back({gNode, gLastPly, gHasLastPly, gAnalysis, gHasAnalysis,
                        gEngineLinesJson, gHasEngineLines});
}

// Serialize candidate lines to JSON, replaying each from `from` so the card
// used and the side to move are correct at every ply. Scores are from
// `from`'s side-to-move perspective; `depth` is the search depth.
std::string serializeLines(const Node& from,
                           const std::vector<ScoredLine>& lines, int depth) {
    std::ostringstream out;
    out << "{\"depth\":" << depth << ",\"lines\":[";
    bool firstLine = true;
    for (const auto& sl : lines) {
        if (!firstLine) out << ",";
        firstLine = false;
        out << "{\"score\":" << sl.score << ",\"moves\":[";
        Node n = from;
        bool firstMove = true;
        for (const auto& ply : sl.line) {
            if (!firstMove) out << ",";
            firstMove = false;
            out << "{\"card\":\"" << CurrentCard::name(n.m_cards.card(ply.mCardSlot))
                << "\",\"pass\":" << (ply.mIsPass ? "true" : "false")
                << ",\"fromX\":" << ply.mFrom.mX << ",\"fromY\":" << ply.mFrom.mY
                << ",\"toX\":" << ply.mTo.mX << ",\"toY\":" << ply.mTo.mY
                << ",\"color\":\"" << (n.m_tomove == Piece::Red ? "red" : "blue")
                << "\"}";
            n = n.applyPly(ply);
        }
        out << "]}";
    }
    out << "]}";
    return out.str();
}

Node initialPosition(const Cards& cards, Piece::Color firstPlayer) {
    Node node;
    node.m_cards = cards;
    node.m_tomove = firstPlayer;
    for (int x = 0; x < 5; x++) {
        node.addPiece(Piece(Position(x, 0), Piece::Red, x == 2));  // top row
        node.addPiece(Piece(Position(x, 4), Piece::Blue, x == 2)); // bottom row
    }
    return node;
}

void appendCard(std::ostringstream& out, CurrentCard::CardType type) {
    out << "{\"name\":\"" << CurrentCard::name(type) << "\",\"moves\":[";
    bool first = true;
    for (const auto& m : CurrentCard::movesFor(type)) {
        if (!first) out << ",";
        first = false;
        out << "[" << m.mShiftX << "," << m.mShiftY << "]";
    }
    out << "]}";
}

// Serialize the whole game state for the frontend.
void rebuildJson() {
    gLegal = gNode.legalMoves();

    std::ostringstream out;
    out << "{";

    out << "\"tomove\":\"" << (gNode.m_tomove == Piece::Red ? "red" : "blue") << "\",";

    bool lost = gNode.isLoss();
    out << "\"gameover\":" << (lost ? "true" : "false") << ",";
    out << "\"winner\":\"";
    if (lost) out << (gNode.m_tomove == Piece::Red ? "blue" : "red");
    out << "\",";

    out << "\"pieces\":[";
    bool first = true;
    for (const auto& p : gNode.m_piece) {
        if (!first) out << ",";
        first = false;
        out << "{\"x\":" << p.mPosition.mX << ",\"y\":" << p.mPosition.mY
            << ",\"color\":\"" << (p.mColor == Piece::Red ? "red" : "blue")
            << "\",\"master\":" << (p.mIsMaster ? "true" : "false") << "}";
    }
    out << "],";

    out << "\"cards\":{";
    out << "\"red\":[";
    appendCard(out, gNode.m_cards.card(Cards::CardA1)); out << ",";
    appendCard(out, gNode.m_cards.card(Cards::CardA2));
    out << "],\"blue\":[";
    appendCard(out, gNode.m_cards.card(Cards::CardB1)); out << ",";
    appendCard(out, gNode.m_cards.card(Cards::CardB2));
    out << "],\"side\":";
    appendCard(out, gNode.m_cards.card(Cards::CardExchange));
    out << "},";

    out << "\"legal\":[";
    first = true;
    for (const auto& ply : gLegal) {
        if (!first) out << ",";
        first = false;
        // slot is the absolute card slot; "hand" is 0/1 within the hand
        int hand = ply.mCardSlot - Cards::firstSlot(gNode.m_tomove);
        out << "{\"slot\":" << ply.mCardSlot << ",\"hand\":" << hand
            << ",\"pass\":" << (ply.mIsPass ? "true" : "false")
            << ",\"fromX\":" << ply.mFrom.mX << ",\"fromY\":" << ply.mFrom.mY
            << ",\"toX\":" << ply.mTo.mX << ",\"toY\":" << ply.mTo.mY << "}";
    }
    out << "],";

    out << "\"last\":";
    if (gHasLastPly && !gLastPly.mIsPass) {
        out << "{\"fromX\":" << gLastPly.mFrom.mX << ",\"fromY\":" << gLastPly.mFrom.mY
            << ",\"toX\":" << gLastPly.mTo.mX << ",\"toY\":" << gLastPly.mTo.mY << "}";
    } else {
        out << "null";
    }

    out << "}";
    gJson = out.str();
}


} // namespace

extern "C" {

// Start a new game. firstPlayer: 0 = blue (human), 1 = red (AI), -1 = random.
EMSCRIPTEN_KEEPALIVE
void og_new_game(unsigned seed, int firstPlayer) {
    std::mt19937 rng(seed);
    Cards cards;
    cards.deal(rng);
    Piece::Color first;
    if (firstPlayer == 0) first = Piece::Blue;
    else if (firstPlayer == 1) first = Piece::Red;
    else first = (rng() % 2 == 0) ? Piece::Red : Piece::Blue;
    gNode = initialPosition(cards, first);
    gHasLastPly = false;
    gHasAnalysis = false;
    gHasEngineLines = false;
    gHistory.clear();
    rebuildJson();
}

// Current state (JSON). Valid until the next API call.
EMSCRIPTEN_KEEPALIVE
const char* og_state() {
    return gJson.c_str();
}

// Play the i-th ply of the current legal list. Returns 1 on success.
EMSCRIPTEN_KEEPALIVE
int og_play(int index) {
    if (index < 0 || index >= static_cast<int>(gLegal.size())) return 0;
    if (gNode.isLoss()) return 0;
    pushHistory();
    gLastPly = gLegal.at(index);
    gHasLastPly = true;
    gNode = gNode.applyPly(gLastPly);
    // Position changed by a human move; stale engine analysis no longer applies.
    gHasAnalysis = false;
    gHasEngineLines = false;
    rebuildJson();
    return 1;
}

// Let the engine pick and play a move for the side to move.
// Returns the number of positions searched, or -1 if the game is over.
EMSCRIPTEN_KEEPALIVE
int og_ai_move(int depth) {
    if (gNode.isLoss() || gLegal.empty()) return -1;
    // Multi-PV so we keep the engine's top candidate lines for display. This
    // costs the same as a single-PV search (ordinary alpha-beta either way).
    BestMove search(depth, 3);
    gNode.accept(search);
    if (!search.mHasMove) return -1;

    // Serialize the engine's lines now, from the pre-move position, so the
    // cards and side to move are right. This is the same search behind the
    // score bar, so its first line matches the bar.
    gEngineLinesJson = serializeLines(gNode, search.mLines, depth);
    gHasEngineLines = true;

    pushHistory();
    gLastPly = search.mBest;
    gHasLastPly = true;
    gNode = gNode.applyPly(gLastPly);

    // The engine just searched the pre-move position to `depth`. Its best
    // line, minus the move we just played, is exactly the depth-(depth-1)
    // analysis of the position now on the board, from the new side to move's
    // perspective (hence the negated score). Stash it so og_eval can serve
    // the score bar for free, no extra search.
    gAnalysis.clear();
    gHasAnalysis = false;
    if (!search.mLines.empty()) {
        ScoredLine sl;
        sl.score = -search.mLines.front().score;
        const auto& pv = search.mLines.front().line;
        if (pv.size() > 1)
            sl.line.assign(pv.begin() + 1, pv.end());
        gAnalysis.push_back(std::move(sl));
        gHasAnalysis = true;
    }

    rebuildJson();
    return static_cast<int>(search.mNodes);
}

// Take back the last ply. Returns 1 on success, 0 if there is no history.
EMSCRIPTEN_KEEPALIVE
int og_undo() {
    if (gHistory.empty()) return 0;
    gNode = gHistory.back().node;
    gLastPly = gHistory.back().lastPly;
    gHasLastPly = gHistory.back().hasLastPly;
    gAnalysis = gHistory.back().analysis;     // restore the position's cached line
    gHasAnalysis = gHistory.back().hasAnalysis;
    gEngineLinesJson = gHistory.back().engineLines;
    gHasEngineLines = gHistory.back().hasEngineLines;
    gHistory.pop_back();
    rebuildJson();
    return 1;
}

// Evaluate the current position, returning the best-line score from the side
// to move's perspective. Values of magnitude >= BestMove::WIN are forced
// wins; WIN + d encodes a win reached with d remaining depth.
//
// If useCached is non-zero and the engine already has analysis of this exact
// position (e.g. the line it just searched to make its move), that score is
// returned for free. Otherwise the position is searched `depth` plies ahead
// and the analysis cache is refreshed.
EMSCRIPTEN_KEEPALIVE
double og_eval(int depth, int useCached) {
    if (useCached && gHasAnalysis && !gAnalysis.empty())
        return gAnalysis.front().score;

    BestMove search(depth);
    gNode.accept(search);
    gAnalysis = search.mLines;
    gHasAnalysis = !gAnalysis.empty();
    // Also serialize the lines so og_engine_lines() can return them
    gEngineLinesJson = serializeLines(gNode, search.mLines, depth);
    gHasEngineLines = !gAnalysis.empty();
    return gNode.m_score;
}

// The engine's top candidate lines from the move it last played (the same
// search behind the score bar), as JSON. Valid until the next og_ai_move or
// state change. Empty (no lines) until the engine has moved:
//   {"depth":9,"lines":[{"score":..,"moves":[
//       {"card":"Tiger","pass":false,"fromX":..,"fromY":..,
//        "toX":..,"toY":..,"color":"red"}, ...]}, ...]}
// Scores are from the engine's (mover's) perspective; the frontend negates
// them to show the human's perspective, matching the score bar.
EMSCRIPTEN_KEEPALIVE
const char* og_engine_lines() {
    static const char* kEmpty = "{\"depth\":0,\"lines\":[]}";
    return gHasEngineLines ? gEngineLinesJson.c_str() : kEmpty;
}

// Encode current game state as a hash string (for sharing positions)
EMSCRIPTEN_KEEPALIVE
const char* og_encode_state() {
    static std::string result;
    result = StateEncoding::encodeState(gNode);
    return result.c_str();
}

// Decode a hash string and load the position
EMSCRIPTEN_KEEPALIVE
int og_decode_state(const char* encoded_str) {
    if (!encoded_str) return 0;
    std::string s(encoded_str);
    if (!StateEncoding::decodeState(s, gNode)) return 0;
    gLegal = gNode.legalMoves();
    gHasLastPly = false;
    gHasAnalysis = false;
    gHasEngineLines = false;
    gHistory.clear();
    rebuildJson();
    return 1;
}

} // extern "C"
