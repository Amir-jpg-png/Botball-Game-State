#ifndef TECH_GAME_STATE_PHASE_H
#define TECH_GAME_STATE_PHASE_H
#include <unordered_map>
#include <any>
#include <string>
#include "json.hpp"
#include "GameTableState.h"

using json = nlohmann::json;

class Phase {
    int m_time_to_completion;
    int m_points;
    bool m_done;
    std::string m_id;
    std::vector<std::string> m_allowed_agents;
    std::map<std::string, std::any> m_conditions;
    std::map<std::string, std::any> m_completion;

public:
    explicit Phase(const std::string &phase_name, const json &data);

    void execute(GameTableState &world, const std::function<void()> &action);

    [[nodiscard]] const std::string &get_id() const {
        return m_id;
    }

    [[nodiscard]] int get_time_to_completion() const {
        return m_time_to_completion;
    }

    [[nodiscard]] const std::vector<std::string> &get_allowed_agents() const {
        return m_allowed_agents;
    }

    [[nodiscard]] const std::map<std::string, std::any> &get_conditions() const {
        return m_conditions;
    }

    [[nodiscard]] const std::map<std::string, std::any> &get_completion() const {
        return m_completion;
    }

    [[nodiscard]] std::map<std::string, std::any> get_completion_copy() const {
        return m_completion;
    }

    [[nodiscard]] bool get_done() const {
        return m_done;
    }

    [[nodiscard]] int get_points() const {
        return m_points;
    }
};

#endif // TECH_GAME_STATE_PHASE_H
