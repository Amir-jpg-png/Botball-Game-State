#include "Server.h"
#include <fstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

char *get_ip() {
    char hostname[256];
    gethostname(hostname, sizeof(hostname));
    const hostent *host_entry = gethostbyname(hostname);
    char *ip = inet_ntoa(*reinterpret_cast<struct in_addr *>(host_entry->h_addr_list[0]));
    return ip;
}

void Server::validate_phase(const Phase &phase) const {
    for (const auto &[key, value]: phase.get_conditions()) {
        if (!m_table_state->has(key)) {
            fatal("error: condition key '" + key + "' of phase '" +
                  phase.get_id() + "' not present in table state", m_log);
        }
        if (m_table_state->get(key).type() != value.type()) {
            fatal("error: type mismatch for condition '" + key +
                  "' in phase '" + phase.get_id() + "'", m_log);
        }
    }

    for (const auto &[key, value]: phase.get_completion()) {
        if (!m_table_state->has(key)) {
            fatal("error: completion key '" + key + "' of phase '" +
                  phase.get_id() + "' not present in table state", m_log);
        }
        if (m_table_state->get(key).type() != value.type()) {
            fatal("error: type mismatch for completion '" + key +
                  "' in phase '" + phase.get_id() + "'", m_log
            );
        }
    }
}

void Server::init(const std::string &table_config_path, const std::string &phase_config_path,
                  const std::string &game_state_config_path) {
    std::ifstream phase_config_file(phase_config_path);
    std::ifstream config_file(game_state_config_path);
    std::ifstream table_config_file(table_config_path);
    m_table_state = std::make_unique<TableState>(table_config_path);
    m_log->info("successfully created TableState");

    if (!phase_config_file.is_open()) {
        fatal("could not open phase config file", m_log);
    }

    if (!config_file.is_open()) {
        fatal("could not open config file", m_log);
    }

    if (!table_config_file.is_open()) {
        fatal("could not open table config file", m_log);
    }

    phase_config_file >> m_phase_data;

    try {
        Phase init_a("INIT_A", m_phase_data.at("INIT_A"));
        Phase init_b("INIT_B", m_phase_data.at("INIT_B"));

        m_phase_state = std::make_unique<PhaseState>(init_a, init_b);
    } catch (std::exception &) {
        fatal("error: Phases INIT_A or INIT_B not found in config", m_log);
    }

    auto &open_phases = m_phase_state->get_open_phases();
    for (const auto &[phase_name, phase]: m_phase_data.items()) {
        open_phases.emplace_back(phase_name, phase);
    }

    phase_config_file.close();
    m_log->info("successfully created PhaseState");

    config_file >> m_config_data;
    m_cfg.Kp = m_config_data.at("points_weight_factor");
    m_cfg.Kt = m_config_data.at("time_weight_factor");
    m_cfg.Kpt = m_config_data.at("potential_weight_factor");
    for (Phase &phase: open_phases) {
        validate_phase(phase);
    }
    config_file.close();
    m_log->info("successfully created Config (GameState Config)");

    table_config_file >> m_table_data;
    table_config_file.close();
}

json Server::generate_response() const {
    json resp;
    resp["type"] = "FULL_STATE";
    resp["table"] = m_table_data;
    resp["phases"] = m_phase_data;
    resp["game_config"] = m_config_data;
    return resp;
}

GameState Server::serve(const int port) {
    init_server(port);
    m_log->info("listening on: {}:{}", get_ip(), port);
    sockaddr_in client_addr{};
    socklen_t client_len = sizeof(client_addr);

    int listen_fd = m_fd;
    m_fd = accept(m_fd, reinterpret_cast<sockaddr *>(&client_addr), &client_len);
    if (m_fd < 0)
        fatal("error: failed to accept connection", m_log);

    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, ip, sizeof(ip));
    uint16_t client_port = ntohs(client_addr.sin_port);
    m_log->info("accepted connection from {}:{}", ip, client_port);

    std::optional<json> msg_optional = recv_json();
    if (!msg_optional.has_value()) {
        fatal("failed to receive request from client", m_log);
    }
    json msg = msg_optional.value();

    std::string req_type;
    try {
        req_type = msg.at("type");
    } catch (std::exception &) {
        fatal("error: no type field in request body", m_log);
    }


    if (req_type == "REQUEST_STATE") {
        send_json(generate_response());
    } else {
        fatal("error: unknown request type", m_log);
    }
    close(listen_fd);
    auto gs = GameState(*m_table_state, m_cfg, *m_phase_state);
    return gs;
}
