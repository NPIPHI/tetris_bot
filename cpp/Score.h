//
// Created by 16182 on 5/28/2021.
//

#ifndef TETRIS_SCORE_H
#define TETRIS_SCORE_H

#include"Grid.h"
#include"Board.h"
#include<limits>
#include<algorithm>
#include<optional>
#include<numeric>
#include<bit>
#include<cstring>
#include<immintrin.h>
#include<thread>
#include<cassert>
#include<execution>

namespace Score {
    constexpr int line_clear_values[] = {
        0,
        -1000,
        -1000,
        -1000,
        200000,
    };

    constexpr int board_height_values[] = {
        -90000,
        -80000,
        -70000,
        -60000,
        -50000,
        -40000,
        -30000,
        -20000,
        -10000,
        -6000,
        -5000,
        -3000,
        -2000,
        -1000,
        -800,
        -500,
        -400,
        -300,
        -200,
        -100,
        0,
        50,
        100,
        100,
        50,
        0,
        0,
        0,
    };

    std::atomic<size_t> boards_scored{};
    int compute_score(const BoardGrid & grid) {
        boards_scored++;
        auto total_holes = grid.count_holes();
        auto top_occupied = grid.top_occupied();
        auto total_score = total_holes * -500
                + board_height_values[top_occupied]
                + 100 * grid.center_of_mass();
        return total_score;
    }

    int max_y_drop(const BoardGrid & grid, Piece piece, Transfrom transform){
        int min_y = grid.height();
        auto [piece_width, piece_height] = piece.shape(transform.dtheta);
        for(int x = 0; x < piece_width; x++){
            int board_height = grid.column_height(x + transform.dx);
            int piece_bottom_height = piece_height_table[(int)piece.type][transform.dtheta][x];
            min_y = std::min(min_y, board_height + piece_bottom_height - piece_height);
        }
        return min_y;
    }

    std::optional<BoardGrid> drop_piece(const BoardGrid & grid, Piece piece, Transfrom transform){
        auto new_grid = grid;
        auto final_y = max_y_drop(grid, piece, transform);
        if(final_y < 0) return std::nullopt;
        auto [piece_width, piece_height] = piece.shape(transform.dtheta);

        for(int x = 0; x < piece_width; x++){
            for(int y = 0; y < piece_height; y++){
                if(piece.grid().get_rot(x, y, transform.dtheta)){
                    new_grid.set(transform.dx + x, final_y + y, true);
                }
            }
        }

        return new_grid;
    }

    int clear_lines(BoardGrid & grid){
        bool all_good = true;
        for(uint16_t row : grid._mem){
            all_good &= (row == 0x3ff);
        }
        if(all_good) return 0;

        int good_y = grid.height();
        for(int y = grid.height() - 1; y >= 0; y--){
            bool all_full = grid._mem[y] == 0x3ff;
            if(!all_full){
                good_y--;
                grid._mem[good_y] = grid._mem[y];
            }
        }
        for(int y = 0; y < good_y; y++){
            grid._mem[y] = 0;
        }

        return good_y;
    }

    std::pair<Board, int> apply_transform(const Board & board, SwapTransform transform){
        auto piece_shape = transform.swap ? board.held : board.current;
        auto grid = drop_piece(board.grid, piece_shape, transform.transform);
        if(grid.has_value()){
            auto cleared = clear_lines(*grid);
            auto held = transform.swap ? board.current : board.held;
            auto current = board.next_pieces[0];
            std::array<Piece, 5> next_pieces{};
            std::copy(board.next_pieces.begin() + 1, board.next_pieces.end(), next_pieces.begin());
            next_pieces.back() = {PieceType::NONE};
            return {Board(*grid, current, held, next_pieces), cleared};
        } else {
            return {{}, -1};
        }

    }

    int score_move(const Board & board, SwapTransform transform, int depth);
    int score_recur(const Board & board, int depth){
        if(depth <= 1){
            return compute_score(board.grid);
        } else {
            const auto & ctransforms = board.current.legal_transforms();
            const auto & stransforms = board.held.legal_transforms();
            int best_score = std::numeric_limits<int>::min();
            best_score = std::transform_reduce(ctransforms.begin(), ctransforms.end(), best_score,
                                               [](int a, int b){return std::max(a,b);},
                                               [&](auto ctran) -> int {
                                                   return score_move(board, {ctran, false}, depth - 1);
                                               });
            if(board.current == board.held) return best_score;
            best_score = std::transform_reduce(stransforms.begin(), stransforms.end(), best_score,
                                               [](int a, int b){return std::max(a,b);},
                                               [&](auto ctran) -> int {
                                                   return score_move(board, {ctran, true}, depth - 1);
                                               });
            return best_score;
        }
    }

    int score_move(const Board & board, SwapTransform transform, int depth){
        auto [new_board, cleared] = apply_transform(board, transform);
        assert(cleared >= -1 && cleared <= 4);
        if(cleared == -1
           || (*(uint64_t*)&new_board.grid._mem) != 0
           || (*(uint16_t*)&new_board.grid._mem[4]) != 0){
            return std::numeric_limits<int>::min();
        }
        return score_recur(new_board, depth)
        + line_clear_values[cleared]
        + board_height_values[new_board.grid.top_occupied()];
    }

    SwapTransform find_best_move(const Board &board, int depth) {
        const auto & ctransforms = board.current.legal_transforms();
        const auto & stransforms = board.held.legal_transforms();
        std::vector<SwapTransform> all_transforms;
        std::transform(ctransforms.begin(), ctransforms.end(), std::back_inserter(all_transforms), [](auto a) -> SwapTransform {return {a, false};});
        if(board.current != board.held)
            std::transform(stransforms.begin(), stransforms.end(), std::back_inserter(all_transforms), [](auto a) -> SwapTransform {return {a, true};});

        constexpr int thread_pool_size = 16;
        std::vector<std::thread> threads;
        std::vector<int> results(all_transforms.size());
        for(int i = 0; i < thread_pool_size; i++){
            threads.emplace_back([&, i]{
                for(int j = i; j < all_transforms.size(); j += thread_pool_size){
                    results[j] = score_move(board, all_transforms[j], depth);
                }
            });
        }
        for(auto& t: threads) t.join();

        auto best_index = std::max_element(results.begin(), results.end()) - results.begin();
        return all_transforms[best_index];
    }
}

#endif //TETRIS_SCORE_H
