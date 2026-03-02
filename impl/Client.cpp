#include <Client.h>
#include <Phase.h>
#include <PhaseState.h>

#include "TableState.h"


GameState Client::get_remote_state(const std::string &ip_address, const uint16_t port) {
    m_log = create_logger("CLIENT");
    init_client(ip_address, port);
    json req;
    req["type"] = "REQUEST_STATE";
    send_json(req);
    const std::optional<json> resp_optional = recv_json();
    if (!resp_optional.has_value()) {
        fatal("failed to send REQUEST_STATE request, exiting...", m_log);
    }
    return parse_remote_state(resp_optional.value());
}

GameState Client::parse_remote_state(json data) const {
    json table_data;
    try {
        table_data = data.at("table");
    } catch (std::exception &) {
        fatal("error config sent by bot_a does not contain table config", m_log);
    }
    auto table_state = TableState(table_data);
    m_log->info("successfully created TableState");
    json phase_data;

    try {
        phase_data = data.at("phases");
    } catch (std::exception &) {
        fatal("error config sent by bot_a does not contain phase config", m_log);
    }

    std::unique_ptr<PhaseState> phase_state = nullptr;

    try {
        Phase init_a("INIT_A", phase_data.at("INIT_A"));
        Phase init_b("INIT_B", phase_data.at("INIT_B"));
        phase_state = std::make_unique<PhaseState>(init_a, init_b);
    } catch (std::exception &) {
        fatal("error: Phases INIT_A or INIT_B not found in config", m_log);
    }

    auto &open_phases = phase_state->get_open_phases();
    for (const auto &[phase_name, phase]: phase_data.items()) {
        open_phases.emplace_back(phase_name, phase);
    }

    m_log->info("successfully created PhaseState");

    json config_data;
    try {
        config_data = data.at("game_config");
    } catch (std::exception &) {
        fatal("error config sent by bot_a does not contain game state config", m_log);
    }
    Config cfg = {};
    cfg.Kp = config_data.at("points_weight_factor");
    cfg.Kt = config_data.at("time_weight_factor");
    cfg.Kpt = config_data.at("potential_weight_factor");
    m_log->info("successfully created config (GameState config)");
    auto gs = GameState(table_state, cfg, *phase_state);
    return gs;
}
