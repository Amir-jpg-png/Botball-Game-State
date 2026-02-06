#include "GameTableState.h"

#include <fstream>
#include <utility>

#include "json.hpp"

using json = nlohmann::json;

std::any GameTableState::get(const std::string &key) const {
    return m_environment.at(key);
}

void GameTableState::set(const std::string &key, std::any value) {
    m_environment[key] = std::move(value);
}

GameTableState::GameTableState(const std::string &path) {
    std::ifstream file(path);

    if (!file.is_open()) {
        throw std::runtime_error("Could not open world state config file: " + path);
    }

    json data;
    file >> data;

    for (const auto &[fact_name, fact_value]: data.items()) {
        if (fact_value.is_boolean()) {
            m_environment[fact_name] = fact_value.get<bool>();
        } else if (fact_value.is_number_integer()) {
            m_environment[fact_name] = fact_value.get<int>();
        } else if (fact_value.is_number_float()) {
            m_environment[fact_name] = fact_value.get<double>();
        } else if (fact_value.is_string()) {
            m_environment[fact_name] = fact_value.get<std::string>();
        } else if (fact_value.is_array()) {
            if (fact_value.empty()) {
                throw std::runtime_error("Array '" + fact_name + "' is empty; cannot determine type.");
            }

            bool all_bool = std::all_of(fact_value.begin(), fact_value.end(),
                                        [](const json &v) { return v.is_boolean(); });
            bool all_int = std::all_of(fact_value.begin(), fact_value.end(),
                                       [](const json &v) { return v.is_number_integer(); });
            bool all_double = std::all_of(fact_value.begin(), fact_value.end(),
                                          [](const json &v) { return v.is_number_float(); });
            bool all_string = std::all_of(fact_value.begin(), fact_value.end(),
                                          [](const json &v) { return v.is_string(); });

            if (int type_count = all_bool + all_int + all_double + all_string; type_count != 1) {
                throw std::runtime_error("Array '" + fact_name + "' contains mixed types.");
            }

            if (all_bool)
                m_environment[fact_name] = fact_value.get<std::vector<bool> >();
            else if (all_int)
                m_environment[fact_name] = fact_value.get<std::vector<int> >();
            else if (all_double)
                m_environment[fact_name] = fact_value.get<std::vector<double> >();
            else if (all_string)
                m_environment[fact_name] = fact_value.get<std::vector<std::string> >();
        } else {
            throw std::runtime_error("Unsupported JSON type for key: " + fact_name);
        }
    }

    file.close();
}

bool GameTableState::has(const std::string &key) const {
    if (!m_environment.count(key)) {
        return false;
    }
    return true;
}

std::map<std::string, std::any> GameTableState::getAll() const {
    return m_environment;
}
