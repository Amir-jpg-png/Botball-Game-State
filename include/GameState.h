#ifndef GAME_STATE_GAMESTATE_H
#define GAME_STATE_GAMESTATE_H
#include "json.hpp"
#include "PhaseState.h"
#include "GameTableState.h"

class GameState {
    GameTableState m_game_table_model;
    std::unique_ptr<PhaseState> m_phase_state;
    std::string m_agent;

    [[nodiscard]] Phase *get_next_best_phase(const std::string &agent) const;

    [[nodiscard]] double compute_potential(const Phase &phase_candidate) const;

public:
    [[nodiscard]] bool conditions_met(const Phase &phase) const;

    explicit GameState(const std::string &world_config_path, const std::string &phase_config_path);

    void start(const std::string &agent);

    void run(const std::unordered_map<std::string, std::function<void()> > &actions);
};

#endif // GAME_STATE_GAMESTATE_H
