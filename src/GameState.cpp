#include "GameState.h"

#include <fstream>
#include <json.hpp>
#include <utility>
#include <any>

using json = nlohmann::json;

static bool value_satisfies(const std::any &actual, const std::any &required) {
    if (actual.type() != required.type())
        return false;

    // bool: exact match
    if (required.type() == typeid(bool))
        return std::any_cast<bool>(actual) == std::any_cast<bool>(required);

    // int: >=
    if (required.type() == typeid(int))
        return std::any_cast<int>(actual) >= std::any_cast<int>(required);

    // double: >=
    if (required.type() == typeid(double))
        return std::any_cast<double>(actual) >= std::any_cast<double>(required);

    // string: exact match
    if (required.type() == typeid(std::string))
        return std::any_cast<std::string>(actual) == std::any_cast<std::string>(required);

    throw std::runtime_error("Unsupported condition type in value_satisfies()");
}

// TODO: enhance conditions met for arrays
// TODO: write template helpers rather than accepting std::any

GameState::GameState(const std::string &world_config_path, const std::string &phase_config_path)
    : m_game_table_model(world_config_path) {
    std::ifstream file(phase_config_path);

    if (!file.is_open()) {
        throw std::runtime_error("error: could not open phase config file");
    }

    json data;
    file >> data;

    Phase init_a("INIT_A", data.at("INIT_A"));
    Phase init_b("INIT_B", data.at("INIT_B"));

    m_phase_state = std::make_unique<PhaseState>(init_a, init_b);

    auto &open_phases = m_phase_state->get_open_phases();
    for (const auto &[phase_name, phase]: data.items()) {
        open_phases.emplace_back(phase_name, phase);
    }

    file.close();
}


Phase *GameState::get_next_best_phase(const std::string &agent) const {
    Phase *best = nullptr;
    double best_score = -std::numeric_limits<double>::infinity();

    for (Phase &phase: m_phase_state->get_open_phases()) {
        if (phase.get_done())
            continue;

        // Agent permission
        if (std::ranges::find(phase.get_allowed_agents(), agent) == phase.get_allowed_agents().end())
            continue;

        if (!conditions_met(phase))
            continue;

        const double score =
                phase.get_points() - 0.1 * phase.get_time_to_completion() + 0.5 * compute_potential(phase);

        if (score > best_score) {
            best_score = score;
            best = &phase;
        }
    }

    return best;
}

double GameState::compute_potential(const Phase &phase_candidate) const {
    double total_points = 0.0;
    int unlocked_count = 0;

    auto projected = m_game_table_model.getAll();

    for (const auto &[key, value]: phase_candidate.get_completion()) {
        projected[key] = value;
    }

    for (const Phase &phase: m_phase_state->get_open_phases()) {
        if (phase.get_done())
            continue;

        if (&phase == &phase_candidate)
            continue;

        const auto &conditions = phase.get_conditions();

        // Skip phases already unlocked before candidate
        bool already_unlocked = true;
        for (const auto &[key, value]: conditions) {
            if (!value_satisfies(m_game_table_model.get(key), value)) {
                already_unlocked = false;
                break;
            }
        }
        if (already_unlocked)
            continue;

        // Check unlocked after candidate
        bool unlocked = true;
        for (const auto &[key, value]: conditions) {
            if (!value_satisfies(projected.at(key), value)) {
                unlocked = false;
                break;
            }
        }

        if (unlocked) {
            total_points += phase.get_points();
            unlocked_count++;
        }
    }

    return unlocked_count == 0 ? 0.0 : total_points / unlocked_count;
}

bool GameState::conditions_met(const Phase &phase) const {
    for (const auto &[key, required]: phase.get_conditions()) {
        if (!m_game_table_model.has(key))
            return false;

        if (const std::any &actual = m_game_table_model.get(key); !value_satisfies(actual, required))
            return false;
    }
    return true;
}

void GameState::start(const std::string &agent) {
    if (agent == "bot_a") {
        printf("sending gamestate...");
    } else {
        // listen for GameState
        printf("waiting for bot_a...");
    }
    m_agent = agent;
}

void GameState::run(const std::unordered_map<std::string, std::function<void()> > &actions) {
    // IS RUNNING && open_phases != empty
    while (!m_phase_state->get_open_phases().empty()) {
        if (m_agent == "bot_a") {
            auto &phase_a = m_phase_state->get_phase_bot_a();
            if (phase_a.get_done()) {
                const auto *next = get_next_best_phase(m_agent);
                m_phase_state->set_phase_bot_a(*next);
                continue;
            }
            auto &action_a = actions.at(phase_a.get_id());
            phase_a.execute(m_game_table_model, action_a);
        } else if (m_agent == "bot_b") {
            auto &phase_b = m_phase_state->get_phase_bot_b();
            if (phase_b.get_done()) {
                const auto *next = get_next_best_phase(m_agent);
                m_phase_state->set_phase_bot_b(*next);
                continue;
            }
            auto &action_b = actions.at(phase_b.get_id());
            phase_b.execute(m_game_table_model, action_b);
        } else {
            throw std::logic_error("Unknown agent type");
        }
    }
}
