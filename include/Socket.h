#pragma once
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
    void recv_all(void *buf, size_t len);

    /**
     * sends data to a remote socket and keeps sending bytes until all bytes are sent
     * @param buf buffer that bytes get loaded from
     * @param len length of the buffer
     */
    void send_all(const void *buf, size_t len) const;

    /**
     * Sends JSON data to the remote client by following protocol orders
     * 1. send 4 byte containing the size of the JSON data, so the receiver can prepare a big enough buffer
     * 2. send the JSON data
     * @param data to be sent over the socket
     */
    void send_json(const json &data) const;

    /**
     * Receives JSON data from the remote client.
     * 1. Reads in 4 byte determining the size of the JSON data
     * 2. Prepares a buffer and reads in size bytes into the buffer (reads in the JSON data)
     * @return the received JSON data or a nullptr if nothing was sent
     */
    [[nodiscard]] std::optional<json> recv_json();

    /**
     * Shuts the connection down effectively letting the other client know that the connection should be closed now.
     */
    void shutdown_socket();

    /**
     * closes the socket locally
     */
    void close_socket();
};
