#include "GameState.h"

#include <fstream>
#include <json.hpp>

using json = nlohmann::json;

GameState::GameState(const std::string& world_config_path, const std::string& phase_config_path)
    : m_game_table_model(world_config_path) {

    std::ifstream file(phase_config_path);

    if (!file.is_open()) {
        throw std::runtime_error("error: could not open phase config file");
    }

    json data;
    file >> data;

    Phase* start_phases[2];
    for (const auto& [phase_name, phase] : data.items()) {
        if (phase_name == "INIT_A") {
            start_phases[0] = new Phase(phase_name, phase);
        } else if (phase_name == "INIT_B") {
            start_phases[1] = new Phase(phase_name, phase);
        }
        m_phase_registry.emplace_back(new Phase(phase_name, phase));
    }

    m_phase_state = new PhaseState(*start_phases[0], *start_phases[1]);

    file.close();
}
