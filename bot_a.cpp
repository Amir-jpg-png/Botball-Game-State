#include <GameState.h>

int main() {
    auto gs = GameState::connect_server("/tmp/game-state/config/state-config/game-state-config.json",
                                        "/tmp/game-state/config/state-config/table-config.json",
                                        "/tmp/game-state/config/state-config/phase-config.json");

    std::unordered_map<std::string, std::function<void()> > actions;

    actions["INIT_A"] = [&] {
        std::cout << "[ACTION] INIT_A\n";
    };

    actions["INIT_B"] = [&] {
        std::cout << "[ACTION] INIT_B\n";
    };

    actions["CLEAR_DECK"] = [&] {
        std::cout << "[ACTION] CLEAR_DECK\n";
    };

    actions["LONG_TASK"] = [&] {
        std::cout << "[ACTION] LONG_TASK\n";
    };

    gs.run(actions);
}
