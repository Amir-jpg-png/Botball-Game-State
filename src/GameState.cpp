#include "GameState.h"

#include <fstream>
#include <utility>

// Private

static bool value_satisfies(const std::any &actual, const std::any &required) {
    if (actual.type() != required.type())
        return false;

    if (required.type() == typeid(bool))
        return std::any_cast<bool>(actual) == std::any_cast<bool>(required);

    if (required.type() == typeid(int))
        return std::any_cast<int>(actual) == std::any_cast<int>(required);

    if (required.type() == typeid(double))
        return std::any_cast<double>(actual) == std::any_cast<double>(required);

    if (required.type() == typeid(std::string))
        return std::any_cast<std::string>(actual) == std::any_cast<std::string>(required);

    throw std::runtime_error("Unsupported condition type in value_satisfies()");
}

void GameState::update_phase_status(Phase &phase) const {
    if (phase.get_done()) return;
    if (phase.get_status() == TIMEOUT) return;

    phase.set_status(OPEN);

    for (const auto &[key, required]: phase.get_conditions()) {
        if (!value_satisfies(m_game_table_model.get(key), required)) {
            phase.set_status(BLOCKED);
            return;
        }
    }

    if (time_remaining() <= phase.get_time_to_completion()) {
        phase.set_status(TIMEOUT);
    }
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

int GameState::time_remaining() const {
    auto now = std::chrono::steady_clock::now();
    return 120 - duration_cast<std::chrono::seconds>(now - m_game_start).count();
}

Phase GameState::get_next_best_phase() const {
    std::unique_ptr<Phase> best;
    double best_score = -std::numeric_limits<double>::infinity();

    for (Phase &phase: m_phase_state->get_open_phases()) {
        if (phase.get_allowed_agent() != m_agent) {
            continue;
        }

        update_phase_status(phase);
        if (phase.get_status() != OPEN) {
            continue;
        }

        const double score =
                m_config.Kp * phase.get_points() - m_config.Kt * phase.get_time_to_completion() * m_config.time_buffer +
                m_config.Kpt *
                compute_potential(phase);

        if (score > best_score) {
            best_score = score;
            best = std::make_unique<Phase>(phase);
        }
    }

    return *best;
}

// Public

GameState::GameState(const std::string &table_config_path, const std::string &phase_config_path,
                     const std::string &game_state_config_path)
    : m_game_table_model(table_config_path) {
    std::ifstream file(phase_config_path);
    std::ifstream config_file(game_state_config_path);

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

    json config;
    config_file >> config;
    m_config.Kp = config.at("points_weight_factor");
    m_config.Kt = config.at("time_weight_factor");
    m_config.Kpt = config.at("potential_weight_factor");
    m_config.time_buffer = config.at("time_buffer_factor");
}

void GameState::connect(const std::string &agent) {
    if (agent == "bot_a") {
        printf("sending gamestate...");
    } else {
        // listen for GameState
        printf("waiting for bot_a...");
    }
    m_agent = agent;
}

void GameState::run(const std::unordered_map<std::string, std::function<void()> > &actions) {
    m_game_start = std::chrono::steady_clock::now();
    // IS RUNNING && open_phases != empty
    while (!m_phase_state->get_open_phases().empty()) {
        if (m_agent == "bot_a") {
            auto &phase_a = m_phase_state->get_phase_bot_a();
            if (phase_a.get_done()) {
                m_phase_state->remove_phase(phase_a.get_id());
                const auto next = get_next_best_phase();
                m_phase_state->set_phase_bot_a(next);
                continue;
            }
            auto &action_a = actions.at(phase_a.get_id());
            phase_a.execute(m_game_table_model, action_a);
        } else if (m_agent == "bot_b") {
            auto &phase_b = m_phase_state->get_phase_bot_b();
            if (phase_b.get_done()) {
                m_phase_state->remove_phase(phase_b.get_id());
                const auto next = get_next_best_phase();
                m_phase_state->set_phase_bot_b(next);
                continue;
            }
            auto &action_b = actions.at(phase_b.get_id());
            phase_b.execute(m_game_table_model, action_b);
        } else {
            throw std::logic_error("Unknown agent type");
        }
    }
}
