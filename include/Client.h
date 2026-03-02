#ifndef TECH_GAME_STATE_CLIENT_H
#define TECH_GAME_STATE_CLIENT_H
#include <Socket.h>
#include <GameState.h>

class Client : public Socket {
public:
    [[nodiscard]] GameState get_remote_state(const std::string &ip_address, uint16_t port);

    [[nodiscard]] GameState parse_remote_state(json data) const;
};

#endif //TECH_GAME_STATE_CLIENT_H
