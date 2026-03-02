#include "GameState.h"

#include <fstream>
#include <utility>
#include <Server.h>
#include <thread>
#include "Client.h"
#include <mutex>

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

bool GameState::has_phases() {
    for (const auto &phase: m_phase_state.get_open_phases_const()) {
        if (phase.get_allowed_agent() == m_agent) {
            return true;
        }
    }
    return false;
}

void GameState::update_phase_status(Phase &phase) const {
    m_log->trace("updating phase {}...", phase.get_id());
    if (phase.get_done()) {
        m_log->trace("skipped phase {} because it is done", phase.get_id());
        return;
    }
    if (phase.get_status() == TIMEOUT) {
        m_log->trace("skipped phase {} because it timed out", phase.get_id());
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
        m_log->info("phase {} timed out", phase.get_id());
    }
    if (status != phase.get_status()) {
        if (phase.get_status() == OPEN) {
            m_log->info("unlocked phase {}", phase.get_id());
        }
        phase.set_status(phase.get_status(), *m_socket);
    }
    m_log->trace("phase status {}: {}", phase.get_id(), phase.get_status());
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
    m_log->info("potential of phase {}: {}", phase_candidate.get_id(), potential);
    return potential;
}

int GameState::time_remaining() const {
    auto now = std::chrono::steady_clock::now();
    return 120 - duration_cast<std::chrono::seconds>(now - m_game_start).count();
}

std::optional<std::string> GameState::get_next_best_phase() {
    std::unique_ptr<Phase> best;
    double best_score = -std::numeric_limits<double>::infinity();

    m_log->info("get next best phase: {}", m_agent);
    for (Phase &phase: m_phase_state.get_open_phases()) {
        if (phase.get_allowed_agent() != m_agent) {
            m_log->trace("skipped phase {} because agent is not allowed", phase.get_id());
            continue;
        }

        update_phase_status(phase);
        if (phase.get_status() != OPEN) {
            m_log->trace("skipped phase {} because phase is not open", phase.get_id());
            continue;
        }

        const double score =
                m_config.Kp * phase.get_points() - m_config.Kt * phase.get_time_to_completion() +
                m_config.Kpt *
                compute_potential(phase);

        m_log->info("score of phase {}: {}", phase.get_id(), score);
        if (score > best_score) {
            best_score = score;
            best = std::make_unique<Phase>(phase);
        }
    }

    if (!best) {
        m_log->info("no more phases to execute");
        return std::nullopt;
    }
    m_log->info("selected phase {} as next phase", best->get_id());
    return best->get_id();
}

// Public

GameState::GameState(TableState table_state, const Config &config, PhaseState phase_state)
    : m_config(config),
      m_game_table_model(std::move(table_state)),
      m_phase_state(std::move(phase_state)) {
}

GameState GameState::connect_server(const std::string &game_state_config_path,
                                    const std::string &table_state_config_path,
                                    const std::string &phase_state_config_path) {
    auto srv = std::make_unique<Server>();
    srv->init(table_state_config_path, phase_state_config_path, game_state_config_path);

    GameState gs = srv->serve(3000);

    gs.m_socket = std::move(srv);

    gs.m_agent = "bot_a";
    gs.m_log = create_logger("GS");

    return gs;
}

GameState GameState::connect_client(const std::string &ip, const uint16_t port) {
    auto client = std::make_unique<Client>();
    auto gs = client->get_remote_state(ip, port);

    gs.m_socket = std::move(client);
    gs.m_agent = "bot_b";
    gs.m_log = create_logger("GS");
    return gs;
}

void GameState::listen() {
    m_listen_thread = std::thread([this]() {
        while (m_listening) {
            try {
                std::optional<json> data_optional = m_socket->recv_json();
                if (!data_optional.has_value()) {
                    break;
                }
                json data = data_optional.value();
                validate_checksum(data);
                if (data["type"] == "UPDATE_TABLE") {
                    m_log->info("Received update: {}", data["payload"].dump());

                    std::lock_guard lock(m_state_mutex);

                    auto updates = get_key_value(data["payload"]);
                    for (const auto &[key, val]: updates) {
                        m_game_table_model.set(key, val);
                    }
                }
                if (data["type"] == "UPDATE_PHASE") {
                    json payload = data["payload"];
                    for (auto &[key, value]: payload.items()) {
                        if (value.is_number_integer()) {
                            int phaseCode = value.get<int>();
                            const auto status = static_cast<PhaseStatus>(phaseCode);
                            m_log->info("Received update phase [{}]: {}",
                                        key, statusCodeToString(status));

                            {
                                std::lock_guard lock(m_state_mutex);
                                Phase *p = m_phase_state.get_phase_ptr(key);
                                p->set_status(status);
                            }
                        }
                    }
                }
            } catch (const std::exception &) {
                break;
            }
        }
    });
}

void GameState::run(const std::unordered_map<std::string, std::function<void()> > &actions) {
    m_listening = true;
    listen();
    m_game_start = std::chrono::steady_clock::now();

    while (m_listening) {
        std::string active_id;
        {
            std::lock_guard lock(m_state_mutex);
            m_phase_state.clean();

            active_id = (m_agent == "bot_a") ? m_phase_state.get_phase_id_a() : m_phase_state.get_phase_id_b();

            if (active_id.empty() || !m_phase_state.has_phase(active_id) || m_phase_state.get_phase(active_id).
                get_done()) {
                auto next_id = get_next_best_phase();
                if (!next_id) {
                    if (!has_phases()) break;
                } else {
                    active_id = *next_id;
                    if (m_agent == "bot_a") m_phase_state.set_phase_id_a(active_id);
                    else m_phase_state.set_phase_id_b(active_id);
                }
            }
        }

        if (active_id.empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        // Execute action
        auto it = actions.find(active_id);
        if (it != actions.end()) {
            // We need a local copy or a way to access the phase safely
            // Phase::execute handles its own locking for the socket
            Phase *p = nullptr;
            {
                std::lock_guard<std::mutex> lock(m_state_mutex);
                if (m_phase_state.has_phase(active_id)) p = m_phase_state.get_phase_ptr(active_id);
            }

            if (p) {
                p->execute(m_game_table_model, it->second, *m_socket);
            }
        }
    }
    while (true) {
        {
            std::lock_guard lock(m_state_mutex);
            if (m_phase_state.get_open_phases_const().empty()) {
                m_socket->shutdown_socket();
                m_socket->close_socket();
                if (m_listen_thread.joinable()) {
                    m_listen_thread.join();
                }
                break;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
