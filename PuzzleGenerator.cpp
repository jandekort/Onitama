#include "PuzzleGenerator.h"
#include "NodeVisitor.h"
#include <algorithm>
#include <chrono>
#include <random>

PuzzleGenerator::Puzzle PuzzleGenerator::findPuzzle(int depth, int maxPawns, unsigned seed, int timeoutSeconds, int maxRetries)
{
    std::mt19937 rng(seed);

    auto startTime = std::chrono::steady_clock::now();
    auto deadline = startTime + std::chrono::seconds(timeoutSeconds);

    // Milliseconds left in the overall budget. Each verification search is given
    // this as its own cap, so the timeout is a hard limit -- a single deep
    // search can no longer overrun it (previously the budget was only checked
    // *between* searches, and one depth-15 search could run far past it).
    auto remainingMs = [&]() -> long {
        auto left = deadline - std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(left).count();
    };

    for (int attempt = 0; attempt < maxRetries; ++attempt) {
        if (remainingMs() <= 0)
            break;  // Timeout exceeded

        Node node = generateRandomPosition(maxPawns, rng);

        // Play random moves until we find a win-in-N or game ends
        const int maxMoves = 100;
        for (int moveCount = 0; moveCount < maxMoves; ++moveCount) {
            // Require at least depth*2 moves before checking, to ensure a real puzzle
            // Also verify there's no faster mate available
            if (moveCount >= depth * 2 && moveCount % 2 == 0) {
                long budget = remainingMs();
                if (budget <= 0)
                    return {Node(), false};  // Timeout exceeded

                // Cap each search at the time left. If a search is cut short we
                // cannot trust its verdict, and the budget is spent, so stop.
                bool timedOut = false;
                bool winInN = isForcedWinInN(node, depth, budget, timedOut);
                if (timedOut)
                    return {Node(), false};

                if (winInN) {
                    budget = remainingMs();
                    if (budget <= 0)
                        return {Node(), false};
                    bool fasterWin = isForcedWinInN(node, depth - 1, budget, timedOut);
                    if (timedOut)
                        return {Node(), false};

                    // Blue to move and no faster mate: a genuine win-in-N.
                    if (!fasterWin && node.m_tomove == Piece::Blue)
                        return {node, true};
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

    // Failed to find a puzzle after retries or timeout
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

bool PuzzleGenerator::isForcedWinInN(const Node& node, int depth, long budgetMs,
                                    bool& timedOut)
{
    // For win-in-N, search 2N-1 plies: N blue moves + (N-1) red responses
    int searchDepth = 2 * depth - 1;

    // TT + iterative deepening makes the deep win-in-7/8 searches (depth 13/15)
    // feasible. This runs in a tight generation loop and allocates a fresh
    // table per call, so the table is deliberately right-sized (~256k entries,
    // a few MB) rather than the default ~1M: plenty for these searches without
    // the per-call allocation cost dominating the shallow puzzles.
    SearchConfig cfg = SearchConfig::all();
    cfg.ttEntries = 1u << 18;
    // Cap the search to the budget so it cannot overrun the overall timeout.
    // Must be strictly positive: timeBudgetMs == 0 means "no limit".
    cfg.timeBudgetMs = budgetMs > 0 ? budgetMs : 1;
    BestMove search(searchDepth, 1, cfg);
    Node nodeCopy = node;
    nodeCopy.accept(search);

    timedOut = search.mTimedOut;

    // Check if the score indicates a forced win (>= WIN value)
    return search.mLines.size() > 0 && search.mLines[0].score >= BestMove::WIN;
}
