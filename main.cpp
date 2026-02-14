#include <GameState.h>

int main() {
    GameState gs("/home/pi/gamestate/config/state-config/table-config.json",
                 "/home/pi/gamestate/config/state-config/phase-config.json",
                 "/home/pi/gamestate/config/state-config/game-state-config.json");

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

    gs.connect("bot_a");
    gs.run(actions);

    return 0;
}
