#include "PuzzleGenerator.h"
#include "NodeVisitor.h"
#include <algorithm>
#include <random>

PuzzleGenerator::Puzzle PuzzleGenerator::findPuzzle(int depth, int maxPawns, unsigned seed)
{
    std::mt19937 rng(seed);

    const int maxRetries = 500;

    for (int attempt = 0; attempt < maxRetries; ++attempt) {
        Node node = generateRandomPosition(maxPawns, rng);

        // Play random moves until we find a win-in-N or game ends
        const int maxMoves = 100;
        for (int moveCount = 0; moveCount < maxMoves; ++moveCount) {
            // Require at least depth*2 moves before checking, to ensure a real puzzle
            // Also verify there's no faster mate available
            if (moveCount >= depth * 2 && moveCount % 2 == 0) {
                if (isForcedWinInN(node, depth) && !isForcedWinInN(node, depth - 1)) {
                    // Make sure it's blue's turn (we want blue to win)
                    if (node.m_tomove == Piece::Blue) {
                        return {node, true};
                    }
                }
            }

            // If game is over, abandon this attempt
            if (node.isLoss()) {
                break;
            }

            // Play a random move
            std::vector<Ply> legalMoves = node.legalMoves();
            if (legalMoves.empty()) {
                break;
            }

            std::uniform_int_distribution<int> dist(0, legalMoves.size() - 1);
            int randomIdx = dist(rng);
            node = node.applyPly(legalMoves[randomIdx]);
        }
    }

    // Failed to find a puzzle after retries
    return {Node(), false};
}

Node PuzzleGenerator::generateRandomPosition(int maxPawns, std::mt19937& rng)
{
    Node node;
    node.m_cards.deal(rng);
    node.m_tomove = Piece::Blue;

    // Place blue king at (2, 4) - bottom center
    node.addPiece(Piece(Position(2, 4), Piece::Blue, true));

    // Place red king at (2, 0) - top center
    node.addPiece(Piece(Position(2, 0), Piece::Red, true));

    // Randomly place blue pawns
    std::vector<Position> availablePositions;
    for (int x = 0; x < 5; ++x) {
        for (int y = 0; y < 5; ++y) {
            Position pos(x, y);
            if (pos != Position(2, 4) && pos != Position(2, 0)) {
                availablePositions.push_back(pos);
            }
        }
    }

    std::shuffle(availablePositions.begin(), availablePositions.end(), rng);

    int bluePawns = 0;
    int redPawns = 0;

    for (const auto& pos : availablePositions) {
        if (bluePawns < maxPawns && pos.mY >= 2) {
            node.addPiece(Piece(pos, Piece::Blue, false));
            bluePawns++;
        } else if (redPawns < maxPawns && pos.mY < 2) {
            node.addPiece(Piece(pos, Piece::Red, false));
            redPawns++;
        }
    }

    return node;
}

bool PuzzleGenerator::isForcedWinInN(const Node& node, int depth)
{
    // For win-in-N, search 2N-1 plies: N blue moves + (N-1) red responses
    int searchDepth = 2 * depth - 1;
    BestMove search(searchDepth, 1);
    Node nodeCopy = node;
    nodeCopy.accept(search);

    // Check if the score indicates a forced win (>= WIN value)
    return search.mLines.size() > 0 && search.mLines[0].score >= BestMove::WIN;
}
