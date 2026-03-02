#ifndef GAME_STATE_GAMESTATE_H
#define GAME_STATE_GAMESTATE_H
#include <thread>

#include "Util.h"
#include "PhaseState.h"
#include "Socket.h"
#include "TableState.h"
#include <condition_variable>

/**
 * Various configuration options for the GameStates phase choosing algorithm
 */
struct Config {
    double Kp;
    double Kt;
    double Kpt;
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
    std::unique_ptr<Socket> m_socket = nullptr;
    Config m_config{};
    std::thread m_listen_thread;
    mutable std::mutex m_state_mutex;
    std::atomic<bool> m_listening{false};

    [[nodiscard]] bool has_phases() const;

    TableState m_game_table_model;
    PhaseState m_phase_state;
    std::string m_agent;
    std::shared_ptr<spdlog::logger> m_log;

    [[nodiscard]] std::optional<std::string> get_next_best_phase();

    // computes the potential of a phase by adding the points of all phases that would be OPEN after the completion of the phase and dividing them by the amount of phases that get unlocked
    [[nodiscard]] double compute_potential(const Phase &phase_candidate) const;

    // Runs once every time a phase finishes, updates the status of a phase
    void update_phase_status(Phase &phase) const;

    std::chrono::steady_clock::time_point m_game_start;

    // Returns the time remaining for the game (Will probably be replaced by techlib functionality)
    [[nodiscard]] int time_remaining() const;

    void listen();

    void execute_init(const std::function<void()> &action);

public:
    GameState(TableState table_state, const Config &config, PhaseState phase_state);

    GameState(GameState &&other) noexcept;

    /**
     * bot_a calls this function, listens for a request from bot_b and sends the (verified) game state as config file to bot_b\n\n
     * @param table_state_config_path path to table state config file on bot_a (server)
     * @param game_state_config_path path to game state config file on bot_a (server)
     * @param phase_state_config_path path to phase_state_config_path on bot_a (server)
     * @return the game state parsed from the config files
     */
    [[nodiscard]] static GameState connect_server(const std::string &game_state_config_path,
                                                  const std::string &table_state_config_path,
                                                  const std::string &phase_state_config_path);

    /**
     * bot_b calls this function and connects with the listening socket on bot_a, it sends a request for the game state which it receives as JSON and reconstructs it.
     * @param ip specifies ip address of bot_a (only for client)
     * @param port specifies port of bot_a (only for client)
     * @return the reconstructed game state
     */
    [[nodiscard]] static GameState connect_client(const std::string &ip, uint16_t port);

    /**
     * Starts by executing INIT_A and INIT_B and starts calculating, executing and transitioning between phases afterward.
     * @param actions a registry of all functions, each phase needs to have a corresponding function
     */
    void run(const std::unordered_map<std::string, std::function<void()> > &actions);
};

#endif // GAME_STATE_GAMESTATE_H
