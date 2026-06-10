#!/bin/sh
# Build the Puzzle WebAssembly module separately from the game engine
set -e
cd "$(dirname "$0")"

emcc -std=c++14 -O2 \
    PuzzleWebApi.cpp Piece.cpp PieceVisitor.cpp Node.cpp NodeVisitor.cpp \
    TranspositionTable.cpp \
    Position.cpp OnitamaCards.cpp ChessCards.cpp Cards.cpp PuzzleGenerator.cpp StateEncoding.cpp \
    -o docs/puzzle-engine.js \
    -s WASM=1 \
    -s SINGLE_FILE=1 \
    -s ENVIRONMENT=web,worker \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s EXPORTED_FUNCTIONS=_puzzle_find \
    -s EXPORTED_RUNTIME_METHODS=cwrap

echo "Built docs/puzzle-engine.js"
