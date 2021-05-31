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
#include<cassert>

namespace Score {
    constexpr int line_clear_values[] = {
            0,
            -5000,
            0,
            50000,
            200000,
    };

    constexpr int board_height_values[] = {
    -14000,
    -13000,
    -12000,
    -11000,
    -10000,
    -90000,
    -80000,
    -60000,
    -40000,
    -20000,
    -10000,
    -5000,
    -1000,
    -500,
    -300,
    -200,
    -100,
    0,
    400,
    500,
    500,
    500,
    400,
    100,
    -500,
    -1000
    };
    constexpr std::array<uint16_t, 16> scatter(uint16_t b) {
        std::array<uint16_t, 16> ret{};
        for (int i = 0; i < 16; i++) {
            ret[i] = bool((b>>i) & 1);
        }
        return ret;
    }
    constexpr std::array<std::array<uint16_t, 16>, 1 << 10> make_scatter_lookup() {
        std::array<std::array<uint16_t, 16>, 1 << 10> ret{};
        for (int i = 0; i < 1 << 10; i++) {
            ret[i] = scatter(i);
        }
        return ret;
    }
    constexpr std::array<uint8_t, 1<<10> make_popcount_table() {
        std::array<uint8_t, 1<<10> ret{};
        for(unsigned i = 0; i < 1 << 10; i++){
            ret[i] = std::popcount(i);
        }
        return ret;
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
        constexpr static auto popcount_table = make_popcount_table();
        int holes = popcount_table[grid._mem[0]];
        for(int y = 0; y < grid.height() - 1; y++){
            auto row = grid._mem[y] ^ grid._mem[y+1];
            holes += popcount_table[row];
        }
        holes += std::popcount(uint16_t(grid._mem[grid.height() - 1] ^ uint16_t(-1)));
        holes -= 16;
        holes /= 2;
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
        static constexpr auto scatter_lookup_table = make_scatter_lookup();
        static constexpr auto popcount_table = make_popcount_table();
        int total_blocks = 0;
        int total_polar_mass = 0;
        for(int y = 0; y < grid.height(); y++){
            total_blocks += popcount_table[grid._mem[y]];
            total_polar_mass += y * popcount_table[grid._mem[y]];
        }
        if(total_blocks == 0) return 24;
        assert(total_polar_mass / total_blocks == center_of_mass_ref(grid));
        return total_polar_mass / total_blocks;
    }

    size_t total_searched = 0;

    int compute_score(const BoardGrid & grid) {
        total_searched++;
        auto total_holes = count_holes(grid);
        auto top_occupied = get_top_occupied(grid);
        auto total_score = total_holes * -500
                + board_height_values[top_occupied] * 10
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

    int score_recur(const Board & board, int depth){
        if(depth <= 1){
            return compute_score(board.grid);
        } else {
            const auto & ctransforms = board.current.legal_transforms();
            const auto & stransforms = board.held.legal_transforms();
            int best_score = std::numeric_limits<int>::min();
            for(auto ctran : ctransforms){
                auto [new_board, cleared] = apply_transform(board, {ctran, false});
                assert(cleared >= -1 && cleared <= 4);
                if(cleared == -1) continue;
                auto score = score_recur(new_board, depth - 1) + line_clear_values[cleared];
                best_score = std::max(score, best_score);
            }
            for(auto stran : stransforms) {
                auto[new_board, cleared] = apply_transform(board, {stran, true});
                assert(cleared >= -1 && cleared <= 4);
                if(cleared == -1) continue;
                auto score = score_recur(new_board, depth - 1) + line_clear_values[cleared];
                best_score = std::max(score, best_score);
            }
            return best_score;
        }
    }

    SwapTransform find_best_move(const Board &board, int depth) {
        const auto & ctransforms = board.current.legal_transforms();
        const auto & stransforms = board.held.legal_transforms();
        int best_score = std::numeric_limits<int>::min();
        SwapTransform best_move{};
        for(auto ctran : ctransforms){
            auto [new_board, cleared] = apply_transform(board, {ctran, false});
            assert(cleared >= -1 && cleared <= 4);
            if(cleared == -1) continue;

            auto score = score_recur(new_board, depth) + line_clear_values[cleared];
            if(score > best_score){
                best_score = score;
                best_move = {ctran, false};
            }
        }
        for(auto stran : stransforms) {
            auto[new_board, cleared] = apply_transform(board, {stran, true});
            assert(cleared >= -1 && cleared <= 4);
            if(cleared == -1) continue;
            auto score = score_recur(new_board, depth) + line_clear_values[cleared];
            if(score > best_score){
                best_score = score;
                best_move = {stran, true};
            }
        }
        return best_move;
    }
}

#endif //TETRIS_SCORE_H
