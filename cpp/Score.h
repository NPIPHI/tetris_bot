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


    constexpr std::array<uint8_t, 32> popcount_lookup_table_256(){
        std::array<uint8_t, 32> ret{};
        for(uint32_t i = 0; i < 32; i++){
            ret[i] = std::popcount(i%16);
        }
        return ret;
    }

    int popcount_10_6X24(__m256i low16, __m128i high8){
        return std::popcount((uint64_t)_mm256_extract_epi64(low16, 0))
        +std::popcount((uint64_t)_mm256_extract_epi64(low16, 1))
        +std::popcount((uint64_t)_mm256_extract_epi64(low16, 2))
        +std::popcount((uint64_t)_mm256_extract_epi64(low16, 3))
        +std::popcount((uint64_t)_mm_extract_epi64(high8, 0))
        +std::popcount((uint64_t)_mm_extract_epi64(high8, 1));
    }

    int count_holes_ref(const BoardGrid & grid){
        int total_holes = 0;
        for(int x = 0; x < grid.width(); x++){
            bool has_roof = false;
            for(int y = 0; y < grid.height(); y++){
                if(grid.get(x,y)){
                    has_roof = true;
                } else {
                    if(has_roof) total_holes++;
                    has_roof = false;
                }
            }
        }
        return total_holes;
    }
    int count_holes(const BoardGrid & grid){
        __m256i high_chunck_xor1 = _mm256_loadu_si256((const __m256i *)&grid._mem);
        __m256i high_chunck_xor2 = _mm256_loadu_si256((const __m256i *)&grid._mem[1]);
        __m128i low_chunck_xor1 = _mm_loadu_si128((const __m128i *)&grid._mem[16]);
        __m128i low_shifted = _mm_srli_si128(low_chunck_xor1, 2);
        __m128i floor_mask = _mm_set_epi16(0x3ff, 0, 0, 0, 0, 0, 0, 0);
        __m128i low_chunck_xor2 = _mm_or_si128(low_shifted, floor_mask);
        __m256i high = _mm256_xor_si256(high_chunck_xor1, high_chunck_xor2);
        __m128i low = _mm_xor_si128(low_chunck_xor1, low_chunck_xor2);
        int all_popcount = popcount_10_6X24(high, low) + std::popcount(grid._mem[0]);
        int holes = (all_popcount - 10)/2;
        assert(holes == count_holes_ref(grid));
        return holes;
    }

    int get_top_occupied(const BoardGrid & grid){
        for(int y= 0; y < grid.height(); y++){
            if(grid._mem[y] != 0) return y;
        }
        return grid.height();
    }

    int center_of_mass_ref(const BoardGrid & grid){
        int total_polar_mass = 0;
        int total_blocks = 0;
        for(int x = 0; x < grid.width(); x++){
            for(int y = 0; y < grid.height(); y++){
                if(grid.get(x,y)){
                    total_polar_mass+=y;
                    total_blocks++;
                }
            }
        }
        if(total_blocks == 0){
            return grid.height();
        }
        return total_polar_mass/total_blocks;
    }

    int center_of_mass(const BoardGrid & grid){
        const uint64_t * chunck_ptr = (const uint64_t *)(&grid._mem);
        int total_blocks = 0;
        int total_polar_mass = 0;
        for(int y = 0; y < 6; y++){
            auto count = std::popcount(chunck_ptr[y]);
            total_blocks += count;
            total_polar_mass += y * 4 * count;
        }
        if(total_blocks == 0) return 24;
        return total_polar_mass / total_blocks;
    }

    int compute_score(const BoardGrid & grid) {
        auto total_holes = count_holes(grid);
        auto top_occupied = get_top_occupied(grid);
        auto total_score = total_holes * -500
                + board_height_values[top_occupied]
                + 100 * center_of_mass(grid);
        return total_score;
    }

    int get_column_height(const BoardGrid & grid, int x){
        auto column_mask16 = _mm256_set1_epi16(1);
        auto mask16 = _mm256_slli_epi16(column_mask16, x);
        auto chunck16 = _mm256_loadu_si256((const __m256i *)grid._mem.data());
        auto filled16 = _mm256_and_si256(mask16, chunck16);
        auto shifted16 = _mm256_slli_epi16(filled16,15 - x);
        uint32_t set16 = _mm256_movemask_epi8(shifted16);
        if(set16 != 0){
            return std::countr_zero(set16)/2;
        }
        auto column_mask8 = _mm_set1_epi16(1);
        auto mask8 = _mm_slli_epi16(column_mask8, x);
        auto chunck8 = _mm_loadu_si128((const __m128i *)(grid._mem.data()+16));
        auto filled8 = _mm_and_si128(mask8, chunck8);
        auto shifted8 = _mm_slli_epi16(filled8,15 - x);
        uint32_t set8 = _mm_movemask_epi8(shifted8);
        if(set8 != 0){
            return std::countr_zero(set8)/2 + 16;
        }
        return 24;
    }

    int get_column_height_ref(const BoardGrid & grid, int x){
        for(int y = 0; y < grid.height(); y++){
            if(grid.get(x, y)){
                return y;
            }
        }
        return grid.height();
    }

    int max_y_drop(const BoardGrid & grid, Piece piece, Transfrom transform){
        int min_y = grid.height();
        auto [piece_width, piece_height] = piece.shape(transform.dtheta);
        for(int x = 0; x < piece_width; x++){
            int board_height = get_column_height(grid, x + transform.dx);
            assert(board_height == get_column_height_ref(grid,x+transform.dx));
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
        return score_recur(new_board, depth) + line_clear_values[cleared];
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

//        auto move = std::transform_reduce(std::execution::seq, all_transforms.begin(), all_transforms.end(),
//                                          std::pair{std::numeric_limits<int>::min(), SwapTransform{}},
//        [](std::pair<int, SwapTransform> a, std::pair<int, SwapTransform> b){
//            return a.first > b.first ? a : b;
//        },
//        [&](SwapTransform current_transform) -> std::pair<int, SwapTransform> {
//            return {score_move(board, current_transform, depth), current_transform};
//        });
//        if(move.second != all_transforms[best_index]){
//            std::cout << "bad" << std::endl;
//        }
//        return move.second;
    }
}

#endif //TETRIS_SCORE_H
