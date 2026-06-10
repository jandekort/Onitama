#!/bin/sh
# Build the WebAssembly version of Onitama into docs/ (for GitHub Pages).
# Requires Emscripten (brew install emscripten).
set -e
cd "$(dirname "$0")"

emcc -std=c++14 -O2 \
    WebApi.cpp Piece.cpp PieceVisitor.cpp Node.cpp NodeVisitor.cpp \
    TranspositionTable.cpp \
    Position.cpp OnitamaCards.cpp ChessCards.cpp Cards.cpp StateEncoding.cpp \
    -o docs/onitama.js \
    -s WASM=1 \
    -s SINGLE_FILE=1 \
    -s ENVIRONMENT=web,worker \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s EXPORTED_FUNCTIONS=_og_new_game,_og_state,_og_play,_og_ai_move,_og_undo,_og_eval,_og_engine_lines,_og_search_info,_og_best_line,_og_encode_state,_og_decode_state \
    -s EXPORTED_RUNTIME_METHODS=cwrap

echo "Built docs/onitama.js."
echo "The engine now runs in a Web Worker, so serve over HTTP (workers are"
echo "blocked under file://):  (cd docs && python3 -m http.server) then open"
echo "http://localhost:8000/ — GitHub Pages serves it over HTTPS already."
