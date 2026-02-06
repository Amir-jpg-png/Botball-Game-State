#include <iostream>
#include <ostream>

#include "GameState.h"

int main() {
    // Strategy Code
    auto stack_cubes = [&] {
        std::cout << "stacking cubes..." << std::endl;
    };

    auto init_a = [&] {
        std::cout << "initializing a..." << std::endl;
    };

    auto init_b = [&] {
        std::cout << "initializing b..." << std::endl;
    };

    // Initializing phase_actions map
    std::unordered_map<std::string, std::function<void()> > phase_actions;
    phase_actions["STACK_CUBES"] = stack_cubes;
    phase_actions["INIT_A"] = init_a;
    phase_actions["INIT_B"] = init_b;

    // Creating GameState Object as bot_a
    auto game = GameState(
        "/Users/amir/workspace/robotik/TechSupport/ECER2026/game-state/config/state-config/table.json",
        "/Users/amir/workspace/robotik/TechSupport/ECER2026/game-state/config/state-config/phase-config.json");

    // Starting the game
    /// sending game state to bot_b
    /// Setting User Agent to bot_a
    game.start("bot_a");

    // Run Game
    /// iterate over phases
    /// if a phase is completed get the next best phase
    /// repeat until open phases is empty
    game.run(phase_actions);
    // Unclear how to use the game state from bot b
    // IDEA: Different constructor for bot_b
}
