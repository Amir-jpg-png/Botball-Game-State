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


void TableState::set(const std::string &key, std::any value, const Socket &so) {
    // 1. Update local state
    m_environment[key] = value;

    // 2. Prepare the payload (a simple key-value object)
    json payload;
    if (value.type() == typeid(std::string)) {
        payload[key] = std::any_cast<std::string>(value);
    } else if (value.type() == typeid(bool)) {
        payload[key] = std::any_cast<bool>(value);
    } else if (value.type() == typeid(double)) {
        payload[key] = std::any_cast<double>(value);
    } else if (value.type() == typeid(int)) {
        payload[key] = std::any_cast<int>(value);
    } else if (value.type() == typeid(std::vector<std::string>)) {
        payload[key] = std::any_cast<std::vector<std::string> >(value);
    } else if (value.type() == typeid(std::vector<bool>)) {
        payload[key] = std::any_cast<std::vector<bool> >(value);
    } else if (value.type() == typeid(std::vector<double>)) {
        payload[key] = std::any_cast<std::vector<double> >(value);
    } else if (value.type() == typeid(std::vector<int>)) {
        payload[key] = std::any_cast<std::vector<int> >(value);
    } else {
        throw std::runtime_error("Unsupported type in payload");
    }

    // 3. Wrap in the protocol message
    json msg;
    msg["type"] = "UPDATE_TABLE";
    msg["payload"] = payload; // This is now { "init_b": true }
    msg["checksum"] = calculate_checksum(msg["payload"].dump()); // Checksum the payload

    so.send_json(msg);
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
