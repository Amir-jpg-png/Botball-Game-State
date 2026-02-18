#include "TableState.h"

#include <fstream>
#include <utility>

std::any TableState::get(const std::string &key) const {
    const auto it = m_environment.find(key);
    if (it == m_environment.end())
        throw std::logic_error("table state missing key: " + key);
    return it->second;
}

bool TableState::has(const std::string &key) const {
    if (!m_environment.contains(key)) {
        return false;
    }
    return true;
}

void TableState::set(const std::string &key, std::any value) {
    m_environment[key] = std::move(value);
}

TableState::TableState(const std::string &path) {
    m_log = create_logger("TS");
    std::ifstream file(path);

    if (!file.is_open()) {
        fatal("could not open table state file: " + path, m_log);
    }

    json data;
    file >> data;

    m_environment = get_key_value(data);

    file.close();
}

TableState::TableState(const json &data) {
    m_environment = get_key_value(data);
}

std::unordered_map<std::string, std::any> TableState::getAll() const {
    return m_environment;
}
