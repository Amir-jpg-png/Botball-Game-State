#ifndef TECH_GAME_STATE_INCLUDE_H
#define TECH_GAME_STATE_INCLUDE_H
#include <json.hpp>
#include <any>
#include <iostream>

inline void fatal(const std::string &msg) {
    std::cerr << msg << std::endl;
    exit(1);
}

using json = nlohmann::json;

inline std::unordered_map<std::string, std::any> get_key_value(const json &data) {
    std::unordered_map<std::string, std::any> result;
    for (const auto &[condition_key, condition_value]: data.items()) {
        if (condition_value.is_boolean()) {
            result[condition_key] = condition_value.get<bool>();
        } else if (condition_value.is_number_integer()) {
            result[condition_key] = condition_value.get<int>();
        } else if (condition_value.is_number_float()) {
            result[condition_key] = condition_value.get<double>();
        } else if (condition_value.is_string()) {
            result[condition_key] = condition_value.get<std::string>();
        } else if (condition_value.is_array()) {
            if (condition_value.empty()) {
                throw std::runtime_error("Array '" + condition_key + "' is empty; cannot determine type.");
            }

            const bool all_bool = std::ranges::all_of(condition_value,
                                                      [](const json &v) { return v.is_boolean(); });
            const bool all_int = std::ranges::all_of(condition_value,
                                                     [](const json &v) { return v.is_number_integer(); });
            const bool all_double = std::ranges::all_of(condition_value,
                                                        [](const json &v) { return v.is_number_float(); });
            const bool all_string = std::ranges::all_of(condition_value,
                                                        [](const json &v) { return v.is_string(); });

            if (const int type_count = all_bool + all_int + all_double + all_string; type_count != 1) {
                throw std::runtime_error("Array '" + condition_key + "' contains mixed types.");
            }

            if (all_bool)
                result[condition_key] = condition_value.get<std::vector<bool> >();
            else if (all_int)
                result[condition_key] = condition_value.get<std::vector<int> >();
            else if (all_double)
                result[condition_key] = condition_value.get<std::vector<double> >();
            else if (all_string)
                result[condition_key] = condition_value.get<std::vector<std::string> >();
        } else {
            throw std::runtime_error("Unsupported JSON type for key: " + condition_key);
        }
    }
    return result;
}

#endif //TECH_GAME_STATE_INCLUDE_H
