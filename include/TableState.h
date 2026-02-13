#ifndef TECH_GAME_STATE_WORLDSTATE_H
#define TECH_GAME_STATE_WORLDSTATE_H
#include <any>
#include <map>
#include <string>
#include "Util.h"

class GameTableState {
    std::unordered_map<std::string, std::any> m_environment;

public:
    void set(const std::string &key, std::any value);

    [[nodiscard]] bool has(const std::string &key) const;

    [[nodiscard]] std::any get(const std::string &key) const;

    [[nodiscard]] std::unordered_map<std::string, std::any> getAll() const;

    explicit GameTableState(const std::string &path);
};


#endif //TECH_GAME_STATE_WORLDSTATE_H
