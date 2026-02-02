#ifndef TECH_GAME_STATE_WORLDSTATE_H
#define TECH_GAME_STATE_WORLDSTATE_H
#include <any>
#include <map>
#include <string>

class GameTableState {
        std::map<std::string, std::any> m_environment;
        void set(const std::string& key, std::any value);
    public:
        std::any get(const std::string& key);
        explicit GameTableState(const std::string& path);
};


#endif //TECH_GAME_STATE_WORLDSTATE_H