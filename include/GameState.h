//
// Created by Amir on 29.01.26.
//

#ifndef GAME_STATE_GAMESTATE_H
#define GAME_STATE_GAMESTATE_H
#include "json.hpp"
#include "Phase.h"
#include "PhaseState.h"
#include "WorldState.h"

class GameState {
    std::vector<Phase*> m_phase_registry;
    WorldState m_game_table_model;
    PhaseState* m_phase_state;
    public:
    explicit GameState(const std::string& world_config_path, const std::string& phase_config_path);
};

#endif //GAME_STATE_GAMESTATE_H