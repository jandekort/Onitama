cmake_minimum_required(VERSION 3.14)
project(Onitama)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Engine sources, shared by the game executable and the test suite. Keeping
# them in one library means tests link the exact same code the game runs.
add_library(onitama_engine
        Piece.cpp Piece.h
        PieceVisitor.cpp PieceVisitor.h
        Node.cpp Node.h
        NodeVisitor.cpp NodeVisitor.h
        Position.cpp Position.h
        Ply.h
        OnitamaCards.cpp OnitamaCards.h
        ChessCards.cpp ChessCards.h
        DeckConfig.h
        Cards.cpp Cards.h
        PuzzleGenerator.cpp PuzzleGenerator.h
        StateEncoding.cpp StateEncoding.h
        SearchConfig.h
        TranspositionTable.cpp TranspositionTable.h)
target_include_directories(onitama_engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(Onitama main.cpp)
target_link_libraries(Onitama PRIVATE onitama_engine)

enable_testing()
add_subdirectory(tests)
