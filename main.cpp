#include <GameState.h>

int main() {
    GameState gs("/tmp/game-state/config/state-config/table-config.json",
                 "/tmp/game-state/config/state-config/phase-config.json",
                 "/tmp/game-state/config/state-config/game-state-config.json");

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

    actions["WAIT_FOR_INIT"] = [&] {
        std::cout << "[ACTION] WAIT_FOR_INIT\n";
    };

    gs.connect("bot_a");
    gs.run(actions);

    return 0;
}
