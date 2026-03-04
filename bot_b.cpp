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
    actions["COLLECT_DATA"] = [&] {
        std::cout << "[ACTION] COLLECT_DATA" << std::endl;
        std::vector arr = {7, 4, 6};
        gs.mutate_shared_state("collected_data", arr);
        std::cout << "[END ACTION] COLLECT_DATA" << std::endl;
    };
    gs.run(actions);

    return 0;
}
