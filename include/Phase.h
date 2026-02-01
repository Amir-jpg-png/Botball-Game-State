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
    std::vector<std::string> m_allowed_agents;
    std::unordered_map<std::string, std::any> m_conditions;
    std::unordered_map<std::string, std::any> m_completion;
public:
    explicit Phase(const std::string& phase_name, const json& data);
    void on_enter();
    const std::vector<std::string>& get_allowed_agents();
    const std::unordered_map<std::string, std::any>& get_conditions();
    void transition();
};


#endif //TECH_GAME_STATE_PHASE_H