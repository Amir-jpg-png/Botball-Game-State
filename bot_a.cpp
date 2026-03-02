#include <GameState.h>

int main() {
    // auto gs = GameState::connect_server("/home/pi/state-config/game-state-config.json",
    //                                     "/home/pi/state-config/table-config.json",
    //                                     "/home/pi/state-config/phase-config.json");
    auto gs = GameState::connect_server("/tmp/botball-gamestate/config/state-config/game-state-config.json",
                                        "/tmp/botball-gamestate/config/state-config/table-config.json",
                                        "/tmp/botball-gamestate/config/state-config/phase-config.json");

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

    actions["SOME_BOT_A"] = [&] {
        std::cout << "[ACTION] SOME_BOT_A\n";
    };

    gs.run(actions);
}
