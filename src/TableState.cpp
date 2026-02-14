#include "TableState.h"

#include <fstream>
#include <utility>

std::any TableState::get(const std::string &key) const {
    const auto it = m_environment.find(key);
    if (it == m_environment.end())
        throw std::logic_error("table state missing key: " + key);
    return it->second;
}

void TableState::set(const std::string &key, std::any value) {
    m_environment[key] = std::move(value);
}

TableState::TableState(const std::string &path) {
    std::ifstream file(path);

    if (!file.is_open()) {
        throw std::runtime_error("Could not open table state config file: " + path);
    }

    json data;
    file >> data;

    m_environment = get_key_value_unordered_map(data);

    file.close();
}

std::unordered_map<std::string, std::any> TableState::getAll() const {
    return m_environment;
}
