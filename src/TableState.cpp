#include "GameTableState.h"

#include <fstream>
#include <utility>

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

    m_environment = get_key_value_unordered_map(data);

    file.close();
}

bool GameTableState::has(const std::string &key) const {
    if (!m_environment.count(key)) {
        return false;
    }
    return true;
}

std::unordered_map<std::string, std::any> GameTableState::getAll() const {
    return m_environment;
}
