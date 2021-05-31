//
// Created by 16182 on 5/28/2021.
//

#ifndef TETRIS_BOARD_H
#define TETRIS_BOARD_H

#include"Piece.h"
#include "Grid.h"
#include<array>

struct Board {
    Piece current, held;
    std::array<Piece, 5> next_pieces;
    BoardGrid grid;

    Board(BoardGrid grid, Piece current, Piece held, std::array<Piece,5> next_pieces): current(current), held(held), grid(grid),
                                                                                  next_pieces(next_pieces){
    }

    Board(){

    }

    void print() const{
        for(int y = 0; y < grid.height(); y++){
            for(int x = 0; x < grid.width(); x++){
                std::cout << (grid.get(x,y) ? 'X' : '.');
            }
            std::cout << '\n';
        }
        std::cout.flush();
    }
};


#endif //TETRIS_BOARD_H
