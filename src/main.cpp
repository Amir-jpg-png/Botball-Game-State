#include <iostream>
#include <ostream>

#include "GameState.h"

int main() {

    GameState game();

    // Phases
    auto stack_cubes = [&] {
        std::cout << "stacking cubes..." << std::endl;
    };

    auto init_a = [&] {
        std::cout << "initializing a..." << std::endl;
    };

    auto init_b = [&] {
        std::cout << "initializing b..." << std::endl;
    };




}
