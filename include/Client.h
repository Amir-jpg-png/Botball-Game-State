#pragma once
#include <Socket.h>
#include <GameState.h>

class Client : public Socket {
public:
    /**
     * Initializes the Socket, connects to the server as a client, gets the by the server already verified config and reconstructs the GameState object which it then returns.
     * @param ip_address of the remote server (bot_a)
     * @param port of the remote server
     * @return a GameState object
     */
    [[nodiscard]] GameState get_remote_state(const std::string &ip_address, uint16_t port);

private:
    /**
     * Splits the config and uses it to build the components needed for the GameState (TableState, PhaseState, GameStateConfig), constructs a GameState object and returns it.
     * @param data JSON object containing information about table state, phase state, and game state config
     * @return a GameState object
     */
    [[nodiscard]] GameState parse_remote_state(json data) const;
};
