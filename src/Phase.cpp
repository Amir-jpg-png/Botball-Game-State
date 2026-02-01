#include "Phase.h"

Phase::Phase(const std::string &phase_name, const json &data) {
    m_id = phase_name;
    m_time_to_completion = data.at("timeout").get<int>();
    for (const auto& [condition_key, condition_value] : data.items()) {
        if (condition_value.is_boolean()) {
            m_conditions[condition_key] = condition_value.get<bool>();
        } else if (condition_value.is_number_integer()) {
            m_conditions[condition_key] = condition_value.get<int>();
        } else if (condition_value.is_number_float()) {
            m_conditions[condition_key] = condition_value.get<double>();
        } else if (condition_value.is_string()) {
            m_conditions[condition_key] = condition_value.get<std::string>();
        } else if (condition_value.is_array()) {
            if (condition_value.empty()) {
                throw std::runtime_error("Array '" + condition_key + "' is empty; cannot determine type.");
            }

            const bool all_bool = std::all_of(condition_value.begin(), condition_value.end(), [](const json& v){ return v.is_boolean(); });
            const bool all_int  = std::all_of(condition_value.begin(), condition_value.end(), [](const json& v){ return v.is_number_integer(); });
            const bool all_double = std::all_of(condition_value.begin(), condition_value.end(), [](const json& v){ return v.is_number_float(); });
            const bool all_string = std::all_of(condition_value.begin(), condition_value.end(), [](const json& v){ return v.is_string(); });

            if (const int type_count = all_bool + all_int + all_double + all_string; type_count != 1) {
                throw std::runtime_error("Array '" + condition_key + "' contains mixed types.");
            }

            if (all_bool)
                m_conditions[condition_key] = condition_value.get<std::vector<bool>>();
            else if (all_int)
                m_conditions[condition_key] = condition_value.get<std::vector<int>>();
            else if (all_double)
                m_conditions[condition_key] = condition_value.get<std::vector<double>>();
            else if (all_string)
                m_conditions[condition_key] = condition_value.get<std::vector<std::string>>();
        } else {
            throw std::runtime_error("Unsupported JSON type for key: " + condition_key);
        }
    }
    try {
        m_allowed_agents = data.at("allowed_agents").get<std::vector<std::string>>();
    } catch (json::type_error& e) {
        throw std::runtime_error("error failed to get allowed agents on phase " + m_id + ": " + e.what());
    }

}
