#ifndef TECH_GAME_STATE_PHASE_H
#define TECH_GAME_STATE_PHASE_H
#include <unordered_map>
#include <any>
#include "string"
#include "json.hpp"

using json = nlohmann::json;

class Phase {
    std::string id;
    int time_to_completion;
    std::vector<int> allowed_agents;
    std::unordered_map<std::string, std::any> m_conditions;
    std::unordered_map<std::string, std::any> m_completion;
public:
    explicit Phase(const std::string& phase_name, const json& data);
};


#endif //TECH_GAME_STATE_PHASE_H