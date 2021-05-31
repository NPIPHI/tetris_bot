//
// Created by 16182 on 5/28/2021.
//

#include <cstdlib>
#include <numeric>
#include <vector>

[[gnu::noinline]]
void eat(void*){}

extern "C" int add(int a, int b){
    std::vector<int> v;
    int * mem = (int*)malloc(a*4);
    for(int i = 0; i < a; i++){
        mem[i] = i;
    }
    return std::accumulate(mem, mem + a, 0);
}