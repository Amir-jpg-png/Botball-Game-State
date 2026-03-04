#pragma once
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
    TableState m_game_table;
    PhaseState m_phase_state;
    std::string m_agent;
    std::shared_ptr<spdlog::logger> m_log;
    mutable std::mutex m_state_mutex;
    std::atomic<bool> m_listening{false};
    std::chrono::steady_clock::time_point m_game_start;

    /**
     * Determines whether a bot has any phases left by iterating over all open phases. A bot has phases left if a phase with the same agent exists in the open phases.
     * @return a bool representing if the bot has any phases left
     */
    [[nodiscard]] bool has_phases() const;


    [[nodiscard]] std::optional<std::string> get_next_best_phase();

    /**
     * Gets a phase reference as a parameter and calculates a score by
     * 1. determining how many phases will be unlocked by this phase
     * 2. adding their points together
     * 3. calculating the median
     * @param phase_candidate the phase reference of which the potential needs to be computed
     * @return a double score determining the potential of a phase
     */
    [[nodiscard]] double compute_potential(const Phase &phase_candidate) const;

    /**
     * Updates the status of a phase by
     * 1. Determining whether enough time is left if not -> TIMEOUT
     * 2. Determining if all conditions are met if not -> BLOCKED
     * 3. If the above conditions are met -> OPEN
     * @param phase reference to update the status of
     */
    void update_phase_status(Phase &phase) const;

    /**
     * @return the time remaining until game end as an int
     */
    [[nodiscard]] int time_remaining() const;

    /**
     * starts a thread and listens for incoming requests,
     * does checksum validations and updates the state of the local bot according to the updates sent over the network
     */
    void listen();

    /**
     * Executes the init function of a bot, INIT_A or INIT_B depending on the agent.
     * @param action the function of the bots init phase e.g, the function mapping to INIT_A
     */
    void execute_init(const std::function<void()> &action);

public:
    /**
     *
     * @param table_state
     * @param config
     * @param phase_state
     */
    GameState(TableState table_state, const Config &config, PhaseState phase_state);

    /**
     * Not implemented, declaration necessary so the class can hold a mutex
     * @param other
     */
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
     * Starts by executing the bots init phases then calculates, executes and transitions between phases until all phases are done or timed out.
     * @param actions a registry of all functions, each phase needs to have a corresponding function
     */
    void run(const std::unordered_map<std::string, std::function<void()> > &actions);

    /**
     * Updates a value in the shared game state and synchronizes it with the remote agent.
     * * This method performs a thread-safe update of the local GameTableModel and
     * immediately broadcasts the change to the connected peer via the network socket.
     * * @note This method acquires a lock on m_state_mutex to ensure consistency between
     * the local model and the data sent over the wire.
     * * @param key   The unique string identifier for the state variable (e.g., "collected_data").
     * @param value The data to store, wrapped in a std::any. Ensure the type matches
     * the expected type in the TableState configuration.
     */
    void mutate_shared_state(const std::string &key, std::any value);

    /**
     * Gets a value from the table state for the bot to use, has to be
     * - double
     * - int
     * - bool
     * - std::string
     * - std::vector<double>
     * - std::vector<int>
     * - std::vector<bool>
     * - std::vector<string>
     * @tparam T type of the desired value
     * @param key key holding the desired value
     * @return desired value
     */
    template<typename T>
    T read_shared_state(const std::string &key) const;
};
