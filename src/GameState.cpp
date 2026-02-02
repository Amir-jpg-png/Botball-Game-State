#include "GameState.h"

#include <fstream>
#include <json.hpp>
#include <utility>

using json = nlohmann::json;

GameState::GameState(const std::string &world_config_path, const std::string &phase_config_path,
                     std::unordered_map<std::string, std::function<void()> > phase_actions)
    : m_phase_actions(std::move(phase_actions)),
      m_game_table_model(world_config_path) {
    std::ifstream file(phase_config_path);

    if (!file.is_open()) {
        throw std::runtime_error("error: could not open phase config file");
    }

    json data;
    file >> data;

    Phase init_a("INIT_A", data.at("INIT_A"), m_phase_actions.at("INIT_A"));
    Phase init_b("INIT_B", data.at("INIT_B"), m_phase_actions.at("INIT_B"));

    m_phase_state = new PhaseState(init_a, init_b);

    auto open_phases = m_phase_state->get_completed_phases();
    for (const auto &[phase_name, phase]: data.items()) {
        open_phases.emplace_back(phase_name, phase, m_phase_actions.at(phase_name));
    }

    file.close();
}
