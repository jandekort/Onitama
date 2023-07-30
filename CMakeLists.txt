cmake_minimum_required(VERSION 3.14)
project(Onitama)

set(CMAKE_CXX_STANDARD 14)

add_executable(Onitama main.cpp PieceVisitor.cpp PieceVisitor.h Piece.cpp Piece.h Node.cpp Node.h Position.cpp Position.h Animal.cpp Animal.h Cards.cpp Cards.h NodeVisitor.cpp Nodevisitor.cpp)