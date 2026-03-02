#include <GameState.h>

int main() {
    auto gs = GameState::connect_client("172.17.0.2", 3000);
    std::unordered_map<std::string, std::function<void()> > actions;
    actions["INIT_B"] = [&] {
        std::cout << "[ACTION] INIT_B" << std::endl;
    };
    actions["SOME_BOT_B"] = [&] {
        std::cout << "[ACTION] SOME_BOT_B" << std::endl;
    };
    gs.run(actions);

    return 0;
}
