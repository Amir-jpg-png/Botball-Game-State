#ifndef TECH_GAME_STATE_SOCKET_H
#define TECH_GAME_STATE_SOCKET_H
#include <memory>
#include <netinet/in.h>
#include <spdlog/spdlog.h>
#include "Util.h"

class Socket {
protected:
    int m_fd = -1;
    std::shared_ptr<spdlog::logger> m_log;

private:
    sockaddr_in addr = {};

public:
    /**
     * Opens an IPv4 TCP Socket
     */
    void init_socket(uint16_t port);

    /**
     * Opens an IPv4 TCP Socket, binds it to a specified port and makes it listen to every host
     * @param port to bind to
     */
    void init_server(uint16_t port);

    /**
     * Opens an IPv4 TCP Socket, and connects to a remote socket via ip address and port number
     * @param ip_address of remote socket
     * @param port of remote socket
     */
    void init_client(const std::string &ip_address, uint16_t port);

    /**
     * gets data from a remote socket and keeps reading in until the buffer is full
     * @param fd sockets file descriptor
     * @param buf buffer that bytes get loaded into
     * @param len length of the buffer
     */
    void recv_all(int fd, void *buf, size_t len) const;

    /**
     * sends data to a remote socket and keeps sending bytes until all bytes are sent
     * @param fd sockets file descriptor
     * @param buf buffer that bytes get loaded from
     * @param len length of the buffer
     */
    void send_all(int fd, const void *buf, size_t len) const;

    void send_json(int fd, const json &data) const;

    [[nodiscard]] json recv_json(int fd) const;
};

#endif //TECH_GAME_STATE_SOCKET_H
