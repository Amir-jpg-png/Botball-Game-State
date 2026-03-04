#pragma once
#include <string>

#include "GameState.h"
#include "PhaseState.h"
#include <Socket.h>

class Server : public Socket {
    Config m_cfg = {};
    std::unique_ptr<PhaseState> m_phase_state;
    std::unique_ptr<TableState> m_table_state;
    json m_table_data;
    json m_phase_data;
    json m_config_data;
    std::shared_ptr<spdlog::logger> m_log = create_logger("SERVER");

public:
    /**
     * Reads in all necessary values from the config, constructs PhaseState, TableState and Config object (responsible for configuring the phase selection algorithm).
     * @param table_config_path path to table state config file
     * @param phase_config_path path to phases config file
     * @param game_state_config_path path to game state configuration files
     */
    void init(const std::string &table_config_path, const std::string &phase_config_path,
              const std::string &game_state_config_path);

    /**
     * Validates a phase by comparing key, value pairs against the table state.
     * @param phase to be validated
     */
    void validate_phase(const Phase &phase) const;

    /**
     * Generates the response for bot_b to reconstruct the game state locally.
     */
    [[nodiscard]] json generate_response() const;

    /**
     * listens for a request from bot_b, responds with an acknowledgement, await another request and send the (in init) constructed objects to bot_b
     * @param port to listen on
     */
    [[nodiscard]] GameState serve(int port);
};
