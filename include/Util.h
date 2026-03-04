#pragma once
#include <json.hpp>
#include <any>
#include <iostream>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <list>

namespace Util {
    using json = nlohmann::json;

    /**
     * Utility function to split strings (used for create_logger)
     * @param s string to be split
     * @param delim where to split the string at
     * @return a list of strings each a part of the previous string
     */
    std::list<std::string> split_string(const std::string &s, char delim);

    /**
     * Creates a logger with the desired name configures it and returns it as a shared ptr
     * @param logger_name name of the logger
     * @return a shared ptr to a logger
     */
    std::shared_ptr<spdlog::logger> create_logger(const std::string &logger_name);

    /**
     * Global Variable, a logger that can be used by every one in case no logger is available.
     */
    extern std::shared_ptr<spdlog::logger> LOG;

    /**
     * Logs an error and exits the program with status code 1
     * @param msg error message
     * @param log logger instance for error logging
     */
    [[noreturn]] void fatal(const std::string &msg, const std::shared_ptr<spdlog::logger> &log = LOG);

    /**
     *
     * @param data JSON object to get the key value pairs from
     * @return unordered map of key value pairs
     */
    std::unordered_map<std::string, std::any> get_key_value(const json &data);

    /**
     * calculates a checksum and returns it
     * @param s string to calculate the checksum of
     * @return 4 byte checksum
     */
    uint32_t calculate_checksum(const std::string &s);

    /**
     * Gets a JSON object, takes the payload and calculates its checksum. It then compares the checksum of the object with the locally calculated one and throws if they don't match.
     * @param data JSON object
     * @throws std::runtime_error if the checksums do not match
     */
    void validate_checksum(const json &data);
}
