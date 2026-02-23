#ifndef TECH_GAME_STATE_SOCKET_H
#define TECH_GAME_STATE_SOCKET_H
#include <memory>
#include <netinet/in.h>
#include <spdlog/spdlog.h>
#include "Util.h"

class Socket {
protected:
    int m_fd = -1;
    bool m_closed = true;
    std::shared_ptr<spdlog::logger> m_log;

private:
    sockaddr_in addr = {};

public:
    virtual ~Socket() = default;

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
     * @param buf buffer that bytes get loaded into
     * @param len length of the buffer
     */
    void recv_all(void *buf, size_t len) const;

    /**
     * sends data to a remote socket and keeps sending bytes until all bytes are sent
     * @param buf buffer that bytes get loaded from
     * @param len length of the buffer
     */
    void send_all(const void *buf, size_t len) const;

    void send_json(const json &data) const;

    [[nodiscard]] json recv_json() const;

    void close_socket();

    void shutdown_socket();

    bool get_closed() const;
};

#endif //TECH_GAME_STATE_SOCKET_H
