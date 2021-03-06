//
// Created by 16182 on 5/28/2021.
//

#ifndef TETRIS_BOARD_H
#define TETRIS_BOARD_H

#include"Piece.h"
#include "Grid.h"
#include<array>

struct Constraint {
    int max_height;
    int max_holes;
};

struct Board {
    Piece current, held;
    std::array<Piece, 5> next_pieces;
    BoardGrid grid;

    Board(BoardGrid grid, Piece current, Piece held, std::array<Piece,5> next_pieces): current(current), held(held), grid(grid),
                                                                                  next_pieces(next_pieces){
    }

    Board(): current(), held(), next_pieces(), grid() {}

    [[nodiscard]] bool within_constraint(Constraint c) const {
        return grid.top_occupied() >= c.max_height
            && grid.count_holes() <= c.max_holes;
    }
};


#endif //TETRIS_BOARD_H
