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
    if (status != OPEN && phase.get_status() == OPEN) { m_log->info("unlocked phase {}", phase.get_id()); }
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
        m_log->trace("skipped phase {} because agent is not allowed", phase.get_id());
        if (phase.get_allowed_agent() != m_agent) {
            continue;
        }

        update_phase_status(phase);
        if (phase.get_status() != OPEN) {
            m_log->trace("skipped phase {} because it is not open", phase.get_id());
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
    auto *srv = new Server();
    srv->init(table_state_config_path, phase_state_config_path, game_state_config_path);

    GameState gs = srv->serve(3000);

    gs.m_socket = std::unique_ptr<Socket>(srv);

    gs.m_agent = "bot_a";
    gs.m_log = create_logger("GS");

    return gs;
}

GameState GameState::connect_client(const std::string &ip, const uint16_t port) {
    auto *client = new Client();
    auto gs = client->get_remote_state(ip, port);

    gs.m_socket = std::unique_ptr<Socket>(client);
    gs.m_agent = "bot_b";
    gs.m_log = create_logger("GS");
    return gs;
}

void validate_checksum(const json &data) {
    json data_without_checksum;
    data_without_checksum["type"] = data["type"];
    data_without_checksum["payload"] = data["payload"];
    uint32_t checksum = calculate_checksum(data_without_checksum);
    if (checksum != data["checksum"].get<uint32_t>()) {
        std::cerr << "do something here" << std::endl;
    }
}

void GameState::listen() {
    m_listen_thread = std::thread([this]() {
        while (m_listening) {
            try {
                const json data = m_socket->recv_json();
                if (m_socket->get_closed()) {
                    break;
                }
                if (data["type"] == "UPDATE_TABLE") {
                    // Log the incoming update for debugging
                    m_log->debug("Received update: {}", data["payload"].dump());

                    // Lock to prevent main thread from reading table while we write
                    std::lock_guard lock(m_state_mutex);

                    // get_key_value returns the map of any.
                    // We iterate and update the internal model.
                    auto updates = get_key_value(data["payload"]);
                    for (const auto &[key, val]: updates) {
                        // Use the internal set() that doesn't trigger another broadcast
                        m_game_table_model.set(key, val);
                    }
                }
            } catch (const std::exception &e) {
                if (m_listening) {
                    m_log->error("Listener error: {}", e.what());
                }
                break;
            }
        }
    });
}

void GameState::run(const std::unordered_map<std::string, std::function<void()> > &actions) {
    m_game_start = std::chrono::steady_clock::now();
    m_listening = true;
    listen();

    while (true) {
        // 1. Work with the BOT'S OWN COPY of the phase.
        // This is a member variable, NOT a pointer into a moving vector.
        Phase &bot_phase = (m_agent == "bot_a") ? m_phase_state.get_phase_bot_a() : m_phase_state.get_phase_bot_b();
        std::string current_id = bot_phase.get_id();

        std::unique_lock lock(m_state_mutex);

        // 2. Check if the current phase is done
        if (bot_phase.get_done()) {
            m_phase_state.remove_phase(current_id);
            auto next_id_opt = get_next_best_phase();

            if (!next_id_opt.has_value()) {
                m_log->info("No more phases for {}. Exiting.", m_agent);
                break;
            }

            // Copy the new phase from the master list into the bot's private slot
            Phase next_p = m_phase_state.get_phase(next_id_opt.value());
            if (m_agent == "bot_a") m_phase_state.set_phase_bot_a(next_p);
            else m_phase_state.set_phase_bot_b(next_p);
            continue; // Refresh the 'bot_phase' reference at the top of the loop
        }

        auto action = actions.at(current_id);

        // 3. It is now safe to unlock. 'bot_phase' is a stable member variable.
        lock.unlock();

        // 4. EXECUTE ON THE COPY
        bot_phase.execute(m_game_table_model, action, *m_socket);

        // 5. SYNC: Update the version in the master vector so the other bot/server knows it's done
        lock.lock();
        Phase *master_p = m_phase_state.get_phase_ptr(current_id);
        if (master_p) {
            *master_p = bot_phase;
        }
        lock.unlock();

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    m_socket->shutdown_socket();
    if (m_listen_thread.joinable()) {
        m_listen_thread.join();
        m_log->info("stopping listening thread...");
    }
    m_socket->close_socket();
}
