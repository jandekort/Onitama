# Onitama

A web-based implementation of the board game Onitama, with an AI opponent powered by a minimax search engine.

## Stack

The engine is written in **C++** and compiled to WebAssembly using **Emscripten**, so it runs directly in the browser without a backend server. The frontend is vanilla HTML, CSS, and JavaScript. The engine runs in a Web Worker, so searches never block the UI thread.

## Building

The project uses Emscripten to compile C++ to WebAssembly.

**Prerequisites:**
- C++ compiler (clang/gcc)
- Emscripten (`brew install emscripten` on macOS)

### Game Engine

**To build the main game:**

```sh
./build_web.sh
```

This compiles the C++ engine to WebAssembly and generates `docs/onitama.js` (the engine bundle) and `docs/worker.js` (the worker host). The results are ready to deploy or serve locally.

### Puzzle Generator

**To build the puzzle generator:**

```sh
./build_puzzle.sh
```

This compiles the puzzle generator to WebAssembly and generates `docs/puzzle-engine.js` (the puzzle engine bundle). The puzzle page at `docs/puzzle.html` calls this engine to generate random win-in-N puzzles.

## Playing

### Online

The game is hosted on GitHub Pages:

- **Game**: https://jandekort.github.io/Onitama/
- **Puzzle Generator**: https://jandekort.github.io/Onitama/puzzle.html

Just open either URL in a browser. The engine runs entirely in the browser.

### Locally

To test locally during development, start an HTTP server in the `docs` folder:

```sh
cd docs
python3 -m http.server
```

Then open **http://localhost:8000/** in a browser.

**Why HTTP, not HTTPS?**

The engine runs in a Web Worker, which requires same-origin requests. HTTPS with a self-signed cert (or `localhost` certs) creates trust issues that browsers block — the worker script fails to load. HTTP on `localhost` is unrestricted by browsers for local testing. GitHub Pages serves over HTTPS automatically (it's trusted), so the deployed site has no such problem. For production or external testing, always use HTTPS with a valid certificate.

## Game Rules

- You play blue, moving up the board.
- Select a card, then a piece, then a destination.
- Win by capturing the enemy Master or walking your Master onto the enemy temple square.
- Cards rotate: the card you play is taken by your opponent.

## Features

### Game
- **Configurable engine strength**: 3–11 plies (steps ahead), from Weak to Grandmaster.
- **Evaluation bar**: Shows the score from your perspective (blue at bottom = you're winning).
- **Engine lines**: Displays the AI's top candidate moves and their scores.
- **Take back**: Undo your last move and the AI's reply.
- **Engine off mode**: Play both sides manually for analysis or two-player games.
- **State sharing**: Copy/paste position hashes to share game states.

### Puzzle Generator
- **Random puzzle generation**: Generate win-in-N puzzles (1–4 moves) with configurable piece counts.
- **Forced wins**: Engine verifies that the puzzle has a forced win in exactly N moves.
- **State hashing**: Share puzzle positions with others via encoded state strings.

## Architecture Notes

The engine uses negamax with alpha-beta pruning for move search, and a static evaluation function based on material and piece positioning. Multi-PV (principal variation) collection keeps the top 3 candidate lines with no extra cost. Analysis is cached from the AI's own move search, so the score bar stays responsive.

The UI communicates with the engine via Web Worker RPC: each API call posts a message and resolves when the worker replies.
