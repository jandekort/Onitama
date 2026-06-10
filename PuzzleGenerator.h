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

    static Puzzle findPuzzle(int depth, int maxPawns, unsigned seed);

private:
    static Node generateRandomPosition(int maxPawns, std::mt19937& rng);
    static bool isForcedWinInN(const Node& node, int depth);
};

#endif //ONITAMA_PUZZLEGENERATOR_H
