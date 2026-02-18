#include <Socket.h>
#include <Util.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void Socket::init_socket(const uint16_t port) {
    m_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_fd < 0) fatal("failed to create socket", m_log);

    constexpr int opt = 1;
    setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
}

void Socket::init_server(const uint16_t port) {
    init_socket(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(m_fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0)
        fatal("error: failed to bind socket", m_log);

    if (listen(m_fd, 1) < 0)
        fatal("error: failed to listen on socket");
}

void Socket::init_client(const std::string &ip_address, const uint16_t port) {
    init_socket(port);
    if (ip_address.empty()) {
        fatal("error: client socket can not connect no ip address specified");
    }
    if (inet_pton(AF_INET, ip_address.c_str(), &addr.sin_addr) != 1) {
        fatal("error: invalid ip address specified");
    };
    if (connect(m_fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0) {
        fatal("error: failed to connect to server socket");
    };
}

void Socket::recv_all(const int fd, void *buf, const size_t len) const {
    size_t total = 0;
    while (total < len) {
        const ssize_t n = recv(fd, static_cast<char *>(buf) + total, len - total, 0);
        if (n < 0) fatal("error: connection lost", m_log);
        total += n;
    }
}

void Socket::send_all(const int fd, const void *buf, const size_t len) const {
    size_t total = 0;
    while (total < len) {
        const ssize_t n = send(fd, static_cast<const char *>(buf) + total, len - total, 0);
        if (n < 0) fatal("error: connection lost", m_log);
        total += n;
    }
}

void Socket::send_json(const int fd, const json &data) const {
    const std::string payload = data.dump();
    const uint32_t len = payload.size();
    const uint32_t len_net = htonl(len);
    send_all(fd, &len_net, sizeof(len_net));
    send_all(fd, payload.data(), len);
}

json Socket::recv_json(const int fd) const {
    uint32_t len_net;
    recv_all(fd, &len_net, sizeof(len_net));
    const uint32_t len = ntohl(len_net);

    std::vector<char> buf(len);
    recv_all(fd, buf.data(), len);

    return json::parse(buf.begin(), buf.end());
}
