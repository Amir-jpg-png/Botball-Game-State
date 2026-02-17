#include <GameState.h>

int main() {
    auto gs = GameState::connect("bot_a",
                                 "/tmp/game-state/config/state-config/game-state-config.json",
                                 "/tmp/game-state/config/state-config/table-config.json",
                                 "/tmp/game-state/config/state-config/phase-config.json");

    return 0;
}
