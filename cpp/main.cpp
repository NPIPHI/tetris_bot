
#include <string>
#include <iostream>
#include <map>
#include"Score.h"

//char board_buffer[10*24];
//char * board_head;
//char piece_buffer[7];
//char * piece_head;
//
//extern "C" void begin_stream(){
//    board_head = board_buffer;
//    piece_head = piece_buffer;
//}
//
//extern "C" void write(int i){
//    if(board_head < board_buffer + 240){
//        *board_head = i;
//        board_head++;
//    } else if(piece_head < piece_buffer + 7){
//        *piece_head = i;
//        piece_head++;
//    } else {
////        std::cout << "bad write" << std::endl;
//        std::terminate();
//    }
//}

//extern "C" unsigned get_move(){
//    if(board_head != board_buffer + 10 * 24 || piece_head != piece_buffer + 7) {
////        std::cout << "incomplete write" << std::endl;
//        std::terminate();
//    }
//    Grid g(10, 24);
//    for(int x = 0; x < 10; x++){
//        for(int y = 0; y < 24; y++){
//            g(x, y) = board_buffer[x + y * 10];
//        }
//    }
//    Piece current = {static_cast<PieceType>(piece_buffer[0])};
//    Piece held = {static_cast<PieceType>(piece_buffer[1])};
//    std::array<Piece, 5> next_pieces{};
//    std::transform(piece_buffer + 2, piece_buffer + 7, next_pieces.begin(), [](auto p){return Piece{static_cast<PieceType>(p)};});
//    Board b(std::move(g), current, held, next_pieces);
//    auto move = Score::find_best_move(b, 0);
//    int res = 0;
//    res |= move.transform.dx;
//    res |= move.transform.dtheta << 4;
//    res |= move.swap << 6;
//    return res;
//}

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
    for (int i = 0; i < 1; i++) {
        auto board = Board(grid, current, held, next);
        auto move = Score::find_best_move(board, 4);
        std::cout << move.transform.dx << ':' << move.transform.dtheta << ':' << move.swap << ':' << Score::total_searched << std::endl;
    }
}