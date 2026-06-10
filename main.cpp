#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#include "Node.h"
#include "NodeVisitor.h"
#include "PieceVisitor.h"
#include "DeckConfig.h"
#include "Cards.h"

namespace {

// Square name in chess style: files a-e left to right, ranks 1-5 bottom up.
// Blue's temple arch is c1, Red's is c5.
std::string squareName(const Position& p) {
    std::string s;
    s += static_cast<char>('a' + p.mX);
    s += static_cast<char>('5' - p.mY);
    return s;
}

std::string plyName(const Node& node, const Ply& ply) {
    std::ostringstream out;
    out << CurrentCard::name(node.m_cards.card(ply.mCardSlot));
    if (ply.mIsPass) {
        out << "  (pass - exchange card only)";
    } else {
        out << "  " << squareName(ply.mFrom) << " -> " << squareName(ply.mTo);
        const Piece* victim = node.pieceAt(ply.mTo);
        if (victim) {
            out << "  takes " << (victim->mIsMaster ? "MASTER" : "student");
        }
        if (ply.mTo == Node::temple(Piece::Red)
                && node.pieceAt(ply.mFrom)->mIsMaster
                && node.m_tomove == Piece::Blue)
            out << "  (temple - wins!)";
    }
    return out.str();
}

void showHand(const Cards& cards, Piece::Color color) {
    int firstSlot = Cards::firstSlot(color);
    for (int slot = firstSlot; slot <= firstSlot + 1; slot++) {
        CurrentCard card(cards.card(slot));
        std::cout << "  " << CurrentCard::name(card.mType) << ":" << std::endl;
        card.show();
    }
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

// Ask the human (Blue) to pick one of the legal plies. Returns false on quit.
bool humanPly(const Node& node, const std::vector<Ply>& plies, Ply& chosen) {
    std::cout << "Your cards (you are Blue, moving up the board):" << std::endl;
    showHand(node.m_cards, Piece::Blue);

    std::cout << "Your legal moves:" << std::endl;
    for (size_t i = 0; i < plies.size(); i++)
        std::cout << "  " << i << ": " << plyName(node, plies[i]) << std::endl;

    while (true) {
        std::cout << "Choose a move [0-" << plies.size() - 1 << "] or q to quit: ";
        std::string token;
        if (!(std::cin >> token) || token == "q" || token == "Q")
            return false;
        std::istringstream parse(token);
        size_t choice;
        if (parse >> choice && choice < plies.size()) {
            chosen = plies[choice];
            return true;
        }
        std::cout << "Not a valid choice." << std::endl;
    }
}

} // namespace

int main(int argc, char* argv[]) {

    // Usage: Onitama [depth] [seed] [--selfplay]
    int depth = 5;
    bool selfplay = false;
    unsigned seed = std::random_device{}();
    int numericArg = 0;

    for (int i = 1; i < argc; i++) {
        std::string arg(argv[i]);
        if (arg == "--selfplay") {
            selfplay = true;
        } else {
            std::istringstream parse(arg);
            long value;
            if (parse >> value) {
                if (numericArg == 0) depth = static_cast<int>(value);
                else seed = static_cast<unsigned>(value);
                numericArg++;
            }
        }
    }

    std::mt19937 rng(seed);

    Cards cards;
    cards.deal(rng);

    // Rules.txt: the side card determines the first player. We let it
    // pick one at random.
    Piece::Color firstPlayer = (rng() % 2 == 0) ? Piece::Red : Piece::Blue;

    Node node = initialPosition(cards, firstPlayer);

    std::cout << "=== ONITAMA ===  (search depth " << depth
              << ", seed " << seed << ")" << std::endl;
    std::cout << "The side card assigns the first move to "
              << (firstPlayer == Piece::Red ? "Red (AI)." : "Blue (you).")
              << std::endl << std::endl;

    int moveNumber = 1;
    const int maxMoves = 200;

    while (true) {
        std::cout << node;
        node.m_cards.show();
        std::cout << std::endl;

        if (node.isLoss()) {
            Piece::Color winner = Piece::other(node.m_tomove);
            std::cout << (winner == Piece::Red ? "Red (AI)" : "Blue (you)")
                      << " wins!" << std::endl;
            break;
        }
        if (moveNumber > maxMoves) {
            std::cout << "Move limit reached - game drawn." << std::endl;
            break;
        }

        std::vector<Ply> plies = node.legalMoves();
        Ply chosen;

        if (node.m_tomove == Piece::Red || selfplay) {
            BestMove search(depth);
            node.accept(search);
            chosen = search.mBest;
            std::cout << (node.m_tomove == Piece::Red ? "Red (AI) plays:  "
                                                      : "Blue (AI) plays: ")
                      << plyName(node, chosen)
                      << "   [value " << node.m_score
                      << ", " << search.mNodes << " positions]"
                      << std::endl << std::endl;
        } else {
            if (!humanPly(node, plies, chosen)) {
                std::cout << "Goodbye." << std::endl;
                return 0;
            }
            std::cout << std::endl;
        }

        node = node.applyPly(chosen);
        moveNumber++;
    }

    return 0;
}
