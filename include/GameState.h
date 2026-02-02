#ifndef GAME_STATE_GAMESTATE_H
#define GAME_STATE_GAMESTATE_H
#include "json.hpp"
#include "PhaseState.h"
#include "GameTableState.h"

class GameState {
    std::unordered_map<std::string, std::function<void()> > m_phase_actions;
    GameTableState m_game_table_model;
    PhaseState *m_phase_state;

public:
    explicit GameState(
        const std::string &world_config_path,
        const std::string &phase_config_path,
        std::unordered_map<std::string,
            std::function<void()> > phase_actions);

    void calculate_next_phases();

    void transition();
};

#endif //GAME_STATE_GAMESTATE_H
