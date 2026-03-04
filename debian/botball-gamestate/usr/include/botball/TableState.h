#pragma once
#include "Socket.h"
#include "Util.h"
using namespace Util;

class TableState {
    std::unordered_map<std::string, std::any> m_environment;
    std::shared_ptr<spdlog::logger> m_log;

public:
    explicit TableState(const json &data);

    /**
     * Determines whether a key exists in the TableState if yet return true, else return false
     * @param key to know which value exists or not
     * @return a bool determining whether a value exists or not
     */
    bool has(const std::string &key) const;

    /**
     * Change a value in the TableState
     * @param key of the value to be changed
     * @param value new value
     */
    void set(const std::string &key, std::any value);

    /**
     * Change a value in the TableState and send the change over the network
     * @param key of the value to be changed
     * @param value new value
     */
    void set(const std::string &key, std::any value, const Socket &so);

    /**
     * Gets a value from the table state
     * @param key required to access the value
     * @return the value belonging to the key
     */
    [[nodiscard]] std::any get(const std::string &key) const;

    /**
     * Returns all facts about the table from the table state
     * @return an unordered_map<std::string, std::any>
     */
    [[nodiscard]] std::unordered_map<std::string, std::any> getAll() const;

    /**
     *
     * @param path
     */
    explicit TableState(const std::string &path);
};
