//
// Opt-in search features for BestMove. These are purely additive: the default
// SearchConfig disables every optimization, so the search behaves exactly like
// the original plain alpha-beta and existing callers are unaffected.
//
// The point of routing the features through one small value type is that the
// search code stays simple -- it asks the config what is enabled, the
// optimizations bolt on around the unchanged core, and tests can sweep every
// combination by just constructing different configs.
//

#ifndef ONITAMA_SEARCHCONFIG_H
#define ONITAMA_SEARCHCONFIG_H

#include <cstddef>

struct SearchConfig
{
    // Alpha-beta pruning. On by default (the engine's normal search). Turning
    // it off gives a pure minimax that visits every node -- far slower, but a
    // useful baseline: it returns the same move and score, only the node count
    // changes. Disabling it also disables the transposition table and iterative
    // deepening, which build on alpha-beta bounds.
    bool useAlphaBeta = true;

    // Reuse results for positions reached by different move orders. The table
    // does its own hashing (see TranspositionTable); it never touches the
    // UI-facing StateEncoding.
    bool useTranspositionTable = false;

    // Search depth 1, 2, ... up to the requested depth instead of jumping
    // straight to it. Combined with the transposition table, the shallow
    // passes prime the table so the deepest pass prunes harder.
    bool useIterativeDeepening = false;

    // Transposition table capacity, in number of slots. Rounded up to a power
    // of two so indexing is a mask. Only consulted when the table is enabled.
    std::size_t ttEntries = 1u << 20; // ~1M slots

    // Wall-clock budget for the search, in milliseconds (0 = no limit). Only
    // honored together with iterative deepening, which always has a completed
    // shallower result to return when the clock runs out. The depth fields
    // then act as a ceiling: the search stops at the depth limit or the time
    // limit, whichever comes first.
    long timeBudgetMs = 0;

    // Convenience presets used by callers and tests.
    static SearchConfig plain() { return SearchConfig{}; }

    static SearchConfig minimax()
    {
        SearchConfig c;
        c.useAlphaBeta = false;
        return c;
    }

    static SearchConfig withTT()
    {
        SearchConfig c;
        c.useTranspositionTable = true;
        return c;
    }

    static SearchConfig withID()
    {
        SearchConfig c;
        c.useIterativeDeepening = true;
        return c;
    }

    static SearchConfig all()
    {
        SearchConfig c;
        c.useTranspositionTable = true;
        c.useIterativeDeepening = true;
        return c;
    }

    // Map a UI algorithm selector to a config. The four steps add one
    // optimization at a time, each correctness-neutral:
    //   0 = minimax (no pruning)      1 = alpha-beta
    //   2 = alpha-beta + TT           3 = alpha-beta + TT + iterative deepening
    static SearchConfig fromAlgorithm(int algo)
    {
        switch (algo) {
            case 0:  return minimax();
            case 1:  return plain();   // alpha-beta only
            case 2:  return withTT();  // alpha-beta + TT
            default: return all();     // alpha-beta + TT + ID
        }
    }
};

#endif // ONITAMA_SEARCHCONFIG_H
