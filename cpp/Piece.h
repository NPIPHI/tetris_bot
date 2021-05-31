//
// Created by 16182 on 5/28/2021.
//

#ifndef TETRIS_PIECE_H
#define TETRIS_PIECE_H

#include <array>

enum class PieceType {
    L=0,
    BL=1,
    Z=2,
    BZ=3,
    T=4,
    LINE=5,
    SQUARE=6,
    NONE=7
};

struct Transfrom {
    int dx, dtheta;
};

struct SwapTransform {
    Transfrom transform;
    bool swap;
};

static std::vector<int> unique_rotation_table[]{
    {0,1,2,3},
    {0,1,2,3},
    {0,1},
    {0,1},
    {0,1,2,3},
    {0,1},
    {0}
};

constexpr std::array<int,2> piece_shape_dimensions[]{
        {3,2},
        {3,2},
        {3,2},
        {3,2},
        {3,2},
        {4,1},
        {2,2}
};

std::vector<Transfrom> build_legal_transforms(PieceType type, std::pair<int,int> grid_shape = {10,24}){
    std::vector<Transfrom> transforms;
    for(int theta : unique_rotation_table[(int)type]){
        int width = piece_shape_dimensions[(int)type][theta % 2];
        for(int x = 0; x < grid_shape.first + 1 - width; x++){
            transforms.push_back({x, theta});
        }
    }
    return transforms;
}


static std::vector<Transfrom> legal_transform_table[]{
        build_legal_transforms(PieceType::L),
        build_legal_transforms(PieceType::BL),
        build_legal_transforms(PieceType::Z),
        build_legal_transforms(PieceType::BZ),
        build_legal_transforms(PieceType::T),
        build_legal_transforms(PieceType::LINE),
        build_legal_transforms(PieceType::SQUARE),
};

static std::array<std::vector<int>,4> piece_height_table[]{
        {{{0,0,0},{0,0},{0,1,1},{2,0}}},
        {{{0,0,0},{0,2},{1,1,0},{0,0}}},
        {{{1,0,0},{0,1},{1,0,0},{0,1}}},
        {{{0,0,1},{1,0},{0,0,1},{1,0}}},
        {{{0,0,0},{0,1},{1,0,1},{1,0}}},
        {{{0,0,0,0},{0},{0,0,0,0},{0}}},
        {{{0,0},{0,0},{0,0},{0,0}}}
};

static PieceGrid piece_grid_table[]{
        {{{0,1},{0,1},{1,1}}},
        {{{1,1},{0,1},{0,1}}},
        {{{1,0},{1,1},{0,1}}},
        {{{0,1},{1,1},{1,0}}},
        {{{0,1},{1,1},{0,1}}},
        {{{1},{1},{1},{1}}},
        {{{1,1},{1,1}}}
};

struct Piece {
    PieceType type;
    const std::vector<Transfrom> & legal_transforms() const{
        return legal_transform_table[(int)type];
    }
    const PieceGrid & grid() const{
        return piece_grid_table[(int)type];
    }
    std::pair<int, int> shape(int dtheta) const{
        return {
            piece_shape_dimensions[(int)type][dtheta%2],
            piece_shape_dimensions[(int)type][(dtheta+1)%2],
        };
    }
};


#endif //TETRIS_PIECE_H
