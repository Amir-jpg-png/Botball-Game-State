#ifndef GAME_STATE_GAMESTATE_H
#define GAME_STATE_GAMESTATE_H
#include "Util.h"
#include "PhaseState.h"
#include "TableState.h"

/**
 * Various configuration options for the GameStates phase choosing algorithm
 */
struct Config {
    double Kp;
    double Kt;
    double Kpt;
    double time_buffer;
};

/**
 * Main Application of the competition code\n
 * Responsible for the lifecycle
 * 1. Parse Config files
 * 2. Either listen to or connect to the other bot to synchronize the game state between both of them
 * 3. Run the application, choose phase execute transition...
 * 4. Close all open resources, and write detailed info about the run into logfiles
 */
class GameState {
    Config m_config{};

    TableState m_game_table_model;
    std::unique_ptr<PhaseState> m_phase_state;
    std::string m_agent;
    std::shared_ptr<spdlog::logger> m_logger;

    [[nodiscard]] std::optional<Phase> get_next_best_phase() const;

    // computes the potential of a phase by adding the points of all phases that would be OPEN after the completion of the phase and dividing them by the amount of phases that get unlocked
    [[nodiscard]] double compute_potential(const Phase &phase_candidate) const;

    // Runs once every time a phase finishes, updates the status of a phase
    void update_phase_status(Phase &phase) const;

    std::chrono::steady_clock::time_point m_game_start;

    // Returns the time remaining for the game (Will probably be replaced by techlib functionality)
    [[nodiscard]] int time_remaining() const;

    void validate_phase(const Phase &phase) const;

public:
    explicit GameState(const std::string &table_config_path, const std::string &phase_config_path,
                       const std::string &game_state_config_path);

    /**
     * - bot_a listens for a request from bot_b and sends the entire game state (from init) to bot_b,
     * - bot_b sends a request for the game state and initiates p2p connection on receival
     * @param agent the client ("bot_a" or "bot_b")
     */
    void connect(const std::string &agent);

    /**
     * Starts by executing INIT_A and INIT_B and starts calculating, executing and transitioning between phases afterwards.
     * @param actions a registry of all functions, each phase needs to have a corresponding function
     */
    void run(const std::unordered_map<std::string, std::function<void()> > &actions);
};

#endif // GAME_STATE_GAMESTATE_H
