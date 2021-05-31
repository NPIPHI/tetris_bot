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

struct BoardGrid {
private:
    static constexpr int _width = 10;
    static constexpr int _height = 24;
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
