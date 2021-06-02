cmake_minimum_required(VERSION 3.19)

project(tetris)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
add_executable(findmove main.cpp Board.h Piece.h Grid.h Score.h)