#include "GameState.h"

#include <fstream>
#include <utility>
#include <Server.h>

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
    m_logger->trace("updating phase {}...", phase.get_id());
    if (phase.get_done()) {
        m_logger->trace("skipped phase {} because it is done", phase.get_id());
        return;
    }
    if (phase.get_status() == TIMEOUT) {
        m_logger->trace("skipped phase {} because it timed out", phase.get_id());
        return;
    }
    const PhaseStatus status = phase.get_status();

    phase.set_status(OPEN);

    for (const auto &[key, required]: phase.get_conditions()) {
        if (!value_satisfies(m_game_table_model.get(key), required)) {
            phase.set_status(BLOCKED);
        }
    }

    if (time_remaining() <= phase.get_time_to_completion()) {
        phase.set_status(TIMEOUT);
        m_logger->info("phase {} timed out", phase.get_id());
    }
    if (status != OPEN && phase.get_status() == OPEN) { m_logger->info("unlocked phase {}", phase.get_id()); }
    m_logger->trace("phase status {}: {}", phase.get_id(), phase.get_status());
}

double GameState::compute_potential(const Phase &phase_candidate) const {
    double total_points = 0.0;
    int unlocked_count = 0;

    auto projected = m_game_table_model.getAll();

    for (const auto &[key, value]: phase_candidate.get_completion()) {
        projected[key] = value;
    }

    for (const Phase &phase: m_phase_state.get_open_phases_const()) {
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

    double potential = unlocked_count == 0 ? 0.0 : total_points / unlocked_count;
    m_logger->info("potential of phase {}: {}", phase_candidate.get_id(), potential);
    return potential;
}

int GameState::time_remaining() const {
    auto now = std::chrono::steady_clock::now();
    return 120 - duration_cast<std::chrono::seconds>(now - m_game_start).count();
}

std::optional<Phase> GameState::get_next_best_phase() {
    std::unique_ptr<Phase> best;
    double best_score = -std::numeric_limits<double>::infinity();

    m_logger->info("get next best phase: {}", m_agent);
    for (Phase &phase: m_phase_state.get_open_phases()) {
        m_logger->trace("skipped phase {} because agent is not allowed", phase.get_id());
        if (phase.get_allowed_agent() != m_agent) {
            continue;
        }

        update_phase_status(phase);
        if (phase.get_status() != OPEN) {
            m_logger->trace("skipped phase {} because it is not open", phase.get_id());
            continue;
        }

        const double score =
                m_config.Kp * phase.get_points() - m_config.Kt * phase.get_time_to_completion() +
                m_config.Kpt *
                compute_potential(phase);

        m_logger->info("score of phase {}: {}", phase.get_id(), score);
        if (score > best_score) {
            best_score = score;
            best = std::make_unique<Phase>(phase);
        }
    }

    if (!best) {
        m_logger->info("no more phases to execute");
        return std::nullopt;
    }
    m_logger->info("selected phase {} as next phase", best->get_id());
    return *best;
}

// Public

GameState::GameState(TableState table_state, const Config &config, PhaseState phase_state)
    : m_config(config),
      m_game_table_model(std::move(table_state)),
      m_phase_state(std::move(phase_state)) {
}

GameState GameState::connect(const std::string &agent, const std::string &game_state_config_path,
                             const std::string &table_state_config_path, const std::string &phase_state_config_path) {
    if (agent == "bot_a") {
        Server srv;
        srv.init(table_state_config_path, phase_state_config_path, game_state_config_path);
        // GameState gs = srv.serve(3000);
        // return gs;
        throw std::logic_error("Cannot connect to the server");
    } else {
        fatal("error: agent not in allowed agents ('bot_a', 'bot_b')");
        throw std::logic_error("invalid agent type");
    }
}

void GameState::run(const std::unordered_map<std::string, std::function<void()> > &actions) {
    m_game_start = std::chrono::steady_clock::now();
    // IS RUNNING && open_phases != empty
    while (!m_phase_state.get_open_phases().empty()) {
        if (m_agent == "bot_a") {
            auto &phase_a = m_phase_state.get_phase_bot_a();
            if (phase_a.get_done()) {
                m_logger->info("phase '{}' done", phase_a.get_id());
                m_phase_state.remove_phase(phase_a.get_id());
                if (const auto next = get_next_best_phase(); next.has_value()) {
                    m_phase_state.set_phase_bot_a(next.value());
                    m_logger->info("executing phase {}", next.value().get_id());
                    continue;
                }
                break;
            }
            std::function<void()> action_a;
            try {
                action_a = actions.at(phase_a.get_id());
            } catch (const std::exception &) {
                fatal(
                    "error: action for phase '" + phase_a.get_id() +
                    "' not specified please add function to action registry");
            }
            phase_a.execute(m_game_table_model, action_a);
        } else if (m_agent == "bot_b") {
            auto &phase_b = m_phase_state.get_phase_bot_b();
            if (phase_b.get_done()) {
                m_logger->info("phase '{}' done", phase_b.get_id());
                m_phase_state.remove_phase(phase_b.get_id());
                if (const auto next = get_next_best_phase(); next.has_value()) {
                    m_phase_state.set_phase_bot_b(next.value());
                    m_logger->info("executing phase {}", next.value().get_id());
                    continue;
                }
                break;
            }
            std::function<void()> action_b;
            try {
                action_b = actions.at(phase_b.get_id());
            } catch (const std::exception &) {
                fatal(
                    "error: action for phase '" + phase_b.get_id() +
                    "' not specified please add function to action registry");
            }
            phase_b.execute(m_game_table_model, action_b);
        } else {
            fatal("unknown agent");
        }
    }
}
