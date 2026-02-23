#include "Phase.h"

Phase::Phase(const std::string &phase_id, const json &data) {
    m_id = phase_id;
    m_status = OPEN;
    m_time_to_completion = data.at("timeout").get<int>();
    m_points = data.at("points").get<int>();
    m_conditions = get_key_value(data.at("conditions"));
    m_completion = get_key_value(data.at("completion"));
    try {
        m_allowed_agent = data.at("allowed_agents").get<std::string>();
    } catch (json::type_error &e) {
        throw std::runtime_error("error failed to get allowed agents on phase " + m_id + ": " + e.what());
    }
}

const std::string &Phase::get_id() const {
    return m_id;
}

int Phase::get_time_to_completion() const {
    return m_time_to_completion;
}

const std::string &Phase::get_allowed_agent() const {
    return m_allowed_agent;
}

const std::unordered_map<std::string, std::any> &Phase::get_conditions() const {
    return m_conditions;
}

const std::unordered_map<std::string, std::any> &Phase::get_completion() const {
    return m_completion;
}

bool Phase::get_done() const {
    return m_status == DONE;
}

PhaseStatus Phase::get_status() const {
    return m_status;
}

int Phase::get_points() const {
    return m_points;
}

void Phase::set_status(const PhaseStatus status) {
    m_status = status;
}

void Phase::execute(TableState &table, const std::function<void()> &action, const Socket &so) {
    action();

    for (const auto &[key, value]: m_completion) {
        table.set(key, value, so);
    }

    m_status = DONE;
}
