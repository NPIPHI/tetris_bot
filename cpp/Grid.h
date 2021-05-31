//
// Created by 16182 on 5/28/2021.
//

#ifndef TETRIS_GRID_H
#define TETRIS_GRID_H

#include<vector>
#include<bitset>
#include<array>
#include<cassert>

struct BoardGrid {
private:
    static constexpr int _width = 10;
    static constexpr int _height = 24;
public:
    std::array<uint16_t,24> _mem;
    void set(int x, int y, bool val){
        uint16_t v = val;
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

    bool get_rot(int x, int y, int theta) const {
        switch(theta){
            case 0:
                return get(x,y);
            case 1:
                return get(y,height() - x - 1);
            case 2:
                return get(width() - x - 1, height() - y - 1);
            case 3:
                return get(width() - y - 1, x);
            default:
                return 0;
        }
    }
};

struct PieceGrid {
private:
    std::bitset<6> _mem;
    int _width = 10;
    int _height = 24;
public:
    PieceGrid(const std::vector<std::vector<char>> & contents): _width(contents.size()), _height(contents[0].size()){
        for(int x = 0; x < width(); x++){
            for(int y = 0; y < height(); y++){
                set(x,y, contents[x][y]);
            }
        }
    }
    void set(int x, int y, bool val){
        _mem[x + y * width()] = val;
    }

    bool get(int x, int y)const{
        return _mem[x + y * width()];
    }

    int width() const {
        return _width;
    }

    int height() const {
        return _height;
    }

    bool get_rot(int x, int y, int theta) const {
        switch(theta){
            case 0:
                return get(x,y);
            case 1:
                return get(y,height() - x - 1);
            case 2:
                return get(width() - x - 1, height() - y - 1);
            case 3:
                return get(width() - y - 1, x);
        }
    }
};

//struct Grid {
//private:
//    std::vector<char> _mem;
//    int _width;
//    int _height;
//    const char & const_get(int x, int y) const {
//        return _mem.at(x + y * _width);
//    }
//public:
//    Grid(int width, int height): _width(width), _height(height), _mem(width * height){
//
//    }
//
//    Grid(const std::vector<std::vector<char>> & contents): _width(contents.size()), _height(contents[0].size()){
//        _mem.resize(width() * height());
//        for(int x = 0; x < width(); x++){
//            for(int y = 0; y < height(); y++){
//                (*this)(x,y) = contents[x][y];
//            }
//        }
//    }
//
//    bool get(int x, int y) const {
//        return _mem[x + y * width()];
//    }
//
//    void set(int x, int y, bool val) {
//        _mem[x + y * width()] = val;
//    }
//
//    const char& operator()(int x, int y)const{
//        return const_get(x, y);
//    }
//
//    char& operator()(int x, int y){
//        return const_cast<char&>(const_get(x,y));
//    }
//
//    int width() const{
//        return _width;
//    }
//
//    int height() const{
//        return _height;
//    }
//
//    const char & get_rot(int x, int y, int theta) const {
//        switch(theta){
//            case 0:
//                return (*this)(x,y);
//            case 1:
//                return (*this)(y,height() - x - 1);
//            case 2:
//                return (*this)(width() - x - 1, height() - y - 1);
//            case 3:
//                return (*this)(width() - y - 1, x);
//            default:
//                return _mem[0];
//        }
//    }
//};

#endif //TETRIS_GRID_H
