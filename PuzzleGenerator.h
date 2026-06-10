#ifndef ONITAMA_PUZZLEGENERATOR_H
#define ONITAMA_PUZZLEGENERATOR_H

#include "Node.h"
#include <random>
#include <string>

class PuzzleGenerator
{
public:
    struct Puzzle {
        Node node;
        bool found;
    };

    static Puzzle findPuzzle(int depth, int maxPawns, unsigned seed, int timeoutSeconds = 20, int maxRetries = 500);

private:
    static Node generateRandomPosition(int maxPawns, std::mt19937& rng);

    // True if `node` is a forced win in `depth` for the side to move. The search
    // is capped at budgetMs (so it cannot overrun the generator's overall
    // timeout); timedOut is set when the cap stopped it before a verdict, in
    // which case the boolean result must not be trusted.
    static bool isForcedWinInN(const Node& node, int depth, long budgetMs,
                               bool& timedOut);
};

#endif //ONITAMA_PUZZLEGENERATOR_H
