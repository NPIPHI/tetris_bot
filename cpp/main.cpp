
#include <string>
#include <iostream>
#include <map>
#include"Score.h"

int main(int argc, const char ** argv){
    std::map<std::string, PieceType> piecetype_table {
            {"L", PieceType::L},
            {"bL", PieceType::BL},
            {"Z", PieceType::Z},
            {"bZ", PieceType::BZ},
            {"T", PieceType::T},
            {"line", PieceType::LINE},
            {"square", PieceType::SQUARE},

    };
    if(argc != 9){
        std::cout << "bad argc";
        return 1;
    }
    std::string board_str = argv[1];
    if(board_str.size() != 240){
        std::cout << "bad board_str:" << board_str << std::endl;
        return 1;
    }
    BoardGrid grid{};
    for(int y = 0; y < 24; y++){
        for(int x = 0; x < 10; x++){
            grid.set(x, y, board_str[x + y * 10] == 'X');
        }
    }
    Piece current, held;
    std::array<Piece, 5> next{};
    current = {piecetype_table[argv[2]]};
    held = {piecetype_table[argv[3]]};
    const char ** next_strs = argv + 4;
    for(Piece& piece: next){
        piece = {piecetype_table[*next_strs]};
        next_strs++;
    }
    
    auto board = Board(grid, current, held, next);
    auto move = Score::find_best_move(board, 5);
    std::cout << move.transform.dx << ':' << move.transform.dtheta << ':' << move.swap << ':' << Score::boards_scored << std::endl;
}