#ifndef TECH_GAME_STATE_INCLUDE_H
#define TECH_GAME_STATE_INCLUDE_H
#include <json.hpp>
#include <any>
#include <iostream>
#include <spdlog/sinks/dup_filter_sink.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <list>

using json = nlohmann::json;

inline std::list<std::string> split_string(const std::string &s, char delim) {
    std::list<std::string> items;

    std::string current;
    std::stringstream ss(s);
    while (std::getline(ss, current, delim)) {
        items.push_back(current);
    }

    return items;
}

inline std::shared_ptr<spdlog::logger> create_logger(const std::string &logger_name) {
    std::string new_name = split_string(logger_name, '/').back();
    if (new_name == "left_motor") {
        new_name = "left_m";
    } else if (new_name == "right_motor") {
        new_name = "right_m";
    }

    if (new_name.size() > 8) {
        std::cout << "logger_name" << new_name << " is longer than 8 characters. Please chose a shorter name!";
        new_name = new_name.substr(0, 8);
    }

    // create global logging sinks
    static const std::string pattern = "[%T.%e] %^%=8l%$ %-10n %v";
    static const auto stdout_sink = [&]() {
        auto sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        sink->set_pattern(pattern);
        sink->set_level(spdlog::level::info);
        return sink;
    }();
    static const auto latest_file_sink = [&]() {
        auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("../gamestate-logs/latest.log", true);
        sink->set_pattern(pattern);
        sink->set_level(spdlog::level::trace);
        return sink;
    }();
    static const auto permanent_file_sink = [&]() {
        // get timestamp as string
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S");

        auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("../gamestate-logs/" + oss.str() + ".log");
        sink->set_pattern(pattern);
        sink->set_level(spdlog::level::trace);
        return sink;
    }();

    // create per-logger dup filter sink
    auto dup_sink = std::make_shared<spdlog::sinks::dup_filter_sink_mt>(
        std::chrono::seconds(1)
    );
    dup_sink->set_level(spdlog::level::trace);

    dup_sink->set_level(spdlog::level::trace);
    dup_sink->add_sink(stdout_sink);
    dup_sink->add_sink(latest_file_sink);
    dup_sink->add_sink(permanent_file_sink);

    // initialize logger
    auto logger = std::make_shared<spdlog::logger>("[" + new_name + "]", dup_sink);
    logger->set_level(spdlog::level::trace);
    logger->flush_on(spdlog::level::info);

    return logger;
}

inline std::shared_ptr<spdlog::logger> LOG = create_logger("LOG");

[[noreturn]] inline void fatal(const std::string &msg, const std::shared_ptr<spdlog::logger> &log = LOG) {
    log->error(msg);
    log->flush();
    exit(1);
}

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

inline uint32_t calculate_checksum(const std::string &s) {
    // Simple example using a basic hash (use zlib crc32 for production)
    uint32_t hash = 0x811c9dc5;
    for (const char c: s) {
        hash ^= static_cast<uint8_t>(c);
        hash *= 0x01000193;
    }
    return hash;
}

inline void validate_checksum(const json &data) {
    if (!data.contains("payload") || !data.contains("checksum")) return;

    // Get the payload as a raw string
    std::string payload_str = data["payload"].is_string()
                                  ? data["payload"].get<std::string>()
                                  : data["payload"].dump();

    uint32_t calculated = calculate_checksum(payload_str);
    uint32_t provided = data["checksum"].get<uint32_t>();

    if (calculated != provided) {
        LOG->error("Checksum Mismatch! Calc: {} Provided: {}", calculated, provided);
    }
}

#endif //TECH_GAME_STATE_INCLUDE_H
