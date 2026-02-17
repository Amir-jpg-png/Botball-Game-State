#ifndef TECH_GAME_STATE_TABLE_STATE_H
#define TECH_GAME_STATE_TABLE_STATE_H
#include "Util.h"

class TableState {
    std::unordered_map<std::string, std::any> m_environment;
    std::shared_ptr<spdlog::logger> m_log;

public:
    void set(const std::string &key, std::any value);

    bool has(const std::string &key) const;

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

    explicit TableState(const std::string &path);
};


#endif //TECH_GAME_STATE_TABLE_STATE_H
