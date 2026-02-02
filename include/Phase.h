#ifndef TECH_GAME_STATE_PHASE_H
#define TECH_GAME_STATE_PHASE_H
#include <unordered_map>
#include <any>
#include "string"
#include "json.hpp"

using json = nlohmann::json;

class Phase {
    std::string m_id;
    int m_time_to_completion;
    std::function<void()> m_action;
    std::vector<std::string> m_allowed_agents;
    std::unordered_map<std::string, std::any> m_conditions;
    std::unordered_map<std::string, std::any> m_completion;
public:
    explicit Phase(const std::string &phase_name, const json &data, std::function<void()> action);

    [[nodiscard]] std::string get_id() const {
        return m_id;
    }

    [[nodiscard]] int get_time_to_completion() const {
        return m_time_to_completion;
    }

    void execute() const {
        m_action();
    }

    [[nodiscard]] std::vector<std::string> get_allowed_agents() const {
        return m_allowed_agents;
    }

    [[nodiscard]] std::unordered_map<std::string, std::any> get_conditions() const {
        return m_conditions;
    }

    [[nodiscard]] std::unordered_map<std::string, std::any> get_completion() const {
        return m_completion;
    }
};


#endif //TECH_GAME_STATE_PHASE_H