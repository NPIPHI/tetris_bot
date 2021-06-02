//
// Created by 16182 on 5/28/2021.
//

#ifndef TETRIS_GRID_H
#define TETRIS_GRID_H

#include<vector>
#include<bitset>
#include<array>
#include<iostream>
#include<cassert>
#include<immintrin.h>
#include<bit>

struct BoardGrid {
private:
    static constexpr int _width = 10;
    static constexpr int _height = 24;
    int count_holes_ref() const {
        int total_holes = 0;
        for(int x = 0; x < width(); x++){
            bool has_roof = false;
            for(int y = 0; y < height(); y++){
                if(get(x,y)){
                    has_roof = true;
                } else {
                    if(has_roof) total_holes++;
                    has_roof = false;
                }
            }
        }
        return total_holes;
    }
    static int popcount_10_6X24(__m256i low16, __m128i high8){
        return std::popcount((uint64_t)_mm256_extract_epi64(low16, 0))
               +std::popcount((uint64_t)_mm256_extract_epi64(low16, 1))
               +std::popcount((uint64_t)_mm256_extract_epi64(low16, 2))
               +std::popcount((uint64_t)_mm256_extract_epi64(low16, 3))
               +std::popcount((uint64_t)_mm_extract_epi64(high8, 0))
               +std::popcount((uint64_t)_mm_extract_epi64(high8, 1));
    }


    int center_of_mass_ref() const {
        int total_polar_mass = 0;
        int total_blocks = 0;
        for(int x = 0; x < width(); x++){
            for(int y = 0; y < height(); y++){
                if(get(x,y)){
                    total_polar_mass+=y;
                    total_blocks++;
                }
            }
        }
        if(total_blocks == 0){
            return height();
        }
        return total_polar_mass/total_blocks;
    }

    int column_height_ref(int x) const{
        for(int y = 0; y < height(); y++){
            if(get(x, y)){
                return y;
            }
        }
        return height();
    }

public:
    void print() const{
        for(int y = 0; y < height(); y++){
            for(int x = 0; x < width(); x++){
                std::cout << (get(x,y) ? 'X' : '.');
            }
            std::cout << '\n';
        }
        std::cout.flush();
    }
    std::array<uint16_t,24> _mem;
    void set(int x, int y, bool val){
        uint16_t v = val;
        assert(x <= 10 && y <= 24);
        _mem[y] |= (v << x);
    }

    bool get(int x, int y) const{
        uint16_t mask = 1 << x;
        return _mem[y] & mask;
    }
    static constexpr int width() {
        return _width;
    }

    static constexpr int height() {
        return _height;
    }
    [[nodiscard]] int count_holes() const {
        __m256i high_chunck_xor1 = _mm256_loadu_si256((const __m256i *)&_mem);
        __m256i high_chunck_xor2 = _mm256_loadu_si256((const __m256i *)&_mem[1]);
        __m128i low_chunck_xor1 = _mm_loadu_si128((const __m128i *)&_mem[16]);
        __m128i low_shifted = _mm_srli_si128(low_chunck_xor1, 2);
        __m128i floor_mask = _mm_set_epi16(0x3ff, 0, 0, 0, 0, 0, 0, 0);
        __m128i low_chunck_xor2 = _mm_or_si128(low_shifted, floor_mask);
        __m256i high = _mm256_xor_si256(high_chunck_xor1, high_chunck_xor2);
        __m128i low = _mm_xor_si128(low_chunck_xor1, low_chunck_xor2);

        int all_popcount = popcount_10_6X24(high, low) + std::popcount(_mem[0]);
        int holes = (all_popcount - 10)/2;
        assert(holes == count_holes_ref());
        return holes;
    }

    [[nodiscard]] int top_occupied() const {
        for(int y= 0; y < height(); y++){
            if(_mem[y] != 0) return y;
        }
        return height();
    }


    [[nodiscard]] int center_of_mass() const{
        const uint64_t * chunck_ptr = (const uint64_t *)(&_mem);
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

    [[nodiscard]] int column_height(int x) const {
        auto column_mask16 = _mm256_set1_epi16(1);
        auto mask16 = _mm256_slli_epi16(column_mask16, x);
        auto chunck16 = _mm256_loadu_si256((const __m256i *)_mem.data());
        auto filled16 = _mm256_and_si256(mask16, chunck16);
        auto shifted16 = _mm256_slli_epi16(filled16,15 - x);
        uint32_t set16 = _mm256_movemask_epi8(shifted16);
        if(set16 != 0){
            assert(std::countr_zero(set16)/2 == column_height_ref(x));
            return std::countr_zero(set16)/2;
        }
        auto column_mask8 = _mm_set1_epi16(1);
        auto mask8 = _mm_slli_epi16(column_mask8, x);
        auto chunck8 = _mm_loadu_si128((const __m128i *)(_mem.data()+16));
        auto filled8 = _mm_and_si128(mask8, chunck8);
        auto shifted8 = _mm_slli_epi16(filled8,15 - x);
        uint32_t set8 = _mm_movemask_epi8(shifted8);
        if(set8 != 0){
            assert(std::countr_zero(set8)/2 + 16 == column_height_ref(x));
            return std::countr_zero(set8)/2 + 16;
        }
        assert(height() == column_height_ref(x));
        return height();
    }
};

struct PieceGrid {
private:
    static constexpr int max_dim = 4;
    std::array<std::array<std::array<bool, max_dim>, max_dim>, 4> _rot_mem;
    int _width;
    int _height;
    void set(int x, int y, int which_rot, bool val){
        _rot_mem.at(which_rot).at(x).at(y) = val;
    }
    void make_rot_table(){
        std::array<std::array<int,3>,3> dims = {{{height(), width(), 1}, {width(), height(), 2}, {height(), width(), 3}}};
        for(auto [w,h,t] : dims){
            for(int x = 0; x < w; x++){
                for(int y = 0; y < h; y++){
                    auto val = get_rot_ref(x, y, t);
                    set(x, y, t, val);
                }
            }
        }
    }
    bool get(int x, int y) const {
        return _rot_mem[0].at(x).at(y);
    }
    bool get_rot_ref(int x, int y, int theta) const {
        switch(theta){
            case 0:
                return get(x,y);
            case 1:
                return get(y,height() - x - 1);
            case 2:
                return get(width() - x - 1, height() - y - 1);
            case 3:
                return get(width() - y - 1, x);
            case 4:
                std::terminate();
                return false;
        }
    }

public:
    PieceGrid(const std::vector<std::vector<char>> & contents): _width(contents.size()), _height(contents[0].size()){
        for(int x = 0; x < width(); x++){
            for(int y = 0; y < height(); y++){
                set(x,y, 0, contents[x][y]);
            }
        }
        make_rot_table();
    }

    int width() const {
        return _width;
    }

    int height() const {
        return _height;
    }

    bool get_rot(int x, int y, int theta) const{
        bool val = _rot_mem[theta][x][y];
        assert(val == get_rot_ref(x, y, theta));
        return val;
    }
};


#endif //TETRIS_GRID_H
