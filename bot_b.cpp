#include <GameState.h>

int main() {
    auto gs = GameState::connect_client("172.17.0.3", 3000);
    std::unordered_map<std::string, std::function<void()> > actions;
    actions["INIT_B"] = [&] {
        std::cout << "[ACTION] INIT_B" << std::endl;
    };
    gs.run(actions);

    return 0;
}
