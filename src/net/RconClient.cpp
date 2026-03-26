#include "net/RconClient.h"
#include <cstring>
#include <algorithm>

namespace PSM {

RconClient::RconClient() {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
}

RconClient::~RconClient() {
    disconnect();
#ifdef _WIN32
    WSACleanup();
#endif
}

bool RconClient::connect(const std::string& host, int port, const std::string& password) {
    std::lock_guard<std::mutex> lock(m_mutex);
    disconnect();

    m_host = host;
    m_port = port;
    m_password = password;

    m_socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_socket == INVALID_SOCK) return false;

    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(static_cast<uint16_t>(port));

    if (inet_pton(AF_INET, host.c_str(), &addr.sin_addr) <= 0) {
        // Try hostname resolution
        struct addrinfo hints{}, *result = nullptr;
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;

        if (getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &result) != 0) {
            return false;
        }

        addr = *reinterpret_cast<struct sockaddr_in*>(result->ai_addr);
        freeaddrinfo(result);
    }

    // Set timeout
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&timeout), sizeof(timeout));
    setsockopt(m_socket, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char*>(&timeout), sizeof(timeout));

    if (::connect(m_socket, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) != 0) {
#ifdef _WIN32
        closesocket(m_socket);
#else
        ::close(m_socket);
#endif
        m_socket = INVALID_SOCK;
        return false;
    }

    m_connected = true;

    // Authenticate
    if (!authenticate(password)) {
        disconnect();
        return false;
    }

    return true;
}

void RconClient::disconnect() {
    if (m_socket != INVALID_SOCK) {
#ifdef _WIN32
        closesocket(m_socket);
#else
        ::close(m_socket);
#endif
        m_socket = INVALID_SOCK;
    }
    m_connected = false;
    m_authenticated = false;
}

bool RconClient::isConnected() const {
    return m_connected && m_authenticated;
}

bool RconClient::authenticate(const std::string& password) {
    int32_t authId = ++m_requestId;
    if (!sendPacket(authId, SERVERDATA_AUTH, password)) return false;

    auto resp = receivePacket();
    if (!resp.success) return false;

    // May receive an empty response first
    if (resp.type == SERVERDATA_RESPONSE_VALUE) {
        resp = receivePacket();
    }

    m_authenticated = (resp.requestId == authId && resp.type == SERVERDATA_AUTH_RESPONSE);
    return m_authenticated;
}

RconResponse RconClient::sendCommand(const std::string& command) {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_connected || !m_authenticated) {
        // Try to reconnect
        if (!m_host.empty()) {
            disconnect();
            m_socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (m_socket == INVALID_SOCK) {
                return {0, 0, "", false, "Socket creation failed"};
            }

            struct sockaddr_in addr{};
            addr.sin_family = AF_INET;
            addr.sin_port = htons(static_cast<uint16_t>(m_port));
            inet_pton(AF_INET, m_host.c_str(), &addr.sin_addr);

            struct timeval timeout{5, 0};
            setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&timeout), sizeof(timeout));
            setsockopt(m_socket, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char*>(&timeout), sizeof(timeout));

            if (::connect(m_socket, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) != 0) {
                return {0, 0, "", false, "Connection failed"};
            }

            m_connected = true;
            if (!authenticate(m_password)) {
                return {0, 0, "", false, "Authentication failed"};
            }
        } else {
            return {0, 0, "", false, "Not connected"};
        }
    }

    int32_t cmdId = ++m_requestId;
    if (!sendPacket(cmdId, SERVERDATA_EXECCOMMAND, command)) {
        m_connected = false;
        return {cmdId, 0, "", false, "Send failed"};
    }

    auto resp = receivePacket();
    if (!resp.success) {
        m_connected = false;
    }
    return resp;
}

bool RconClient::sendPacket(int32_t requestId, int32_t type, const std::string& body) {
    int32_t bodyLen = static_cast<int32_t>(body.size());
    int32_t packetSize = 4 + 4 + bodyLen + 1 + 1; // id + type + body + null + null

    std::vector<uint8_t> packet(4 + packetSize);
    auto* ptr = packet.data();

    // Size (not including size field itself)
    memcpy(ptr, &packetSize, 4); ptr += 4;
    // Request ID
    memcpy(ptr, &requestId, 4); ptr += 4;
    // Type
    memcpy(ptr, &type, 4); ptr += 4;
    // Body
    memcpy(ptr, body.c_str(), bodyLen); ptr += bodyLen;
    // Two null terminators
    *ptr++ = 0;
    *ptr++ = 0;

    size_t totalSent = 0;
    while (totalSent < packet.size()) {
        auto sent = send(m_socket, reinterpret_cast<const char*>(packet.data() + totalSent),
                         static_cast<int>(packet.size() - totalSent), 0);
        if (sent <= 0) return false;
        totalSent += sent;
    }

    return true;
}

RconResponse RconClient::receivePacket() {
    RconResponse response;

    // Read size (4 bytes)
    int32_t packetSize = 0;
    int received = recv(m_socket, reinterpret_cast<char*>(&packetSize), 4, 0);
    if (received != 4 || packetSize < 10 || packetSize > 4096 * 4) {
        response.error = "Failed to read packet size";
        return response;
    }

    // Read rest of packet
    std::vector<uint8_t> data(packetSize);
    size_t totalReceived = 0;
    while (totalReceived < static_cast<size_t>(packetSize)) {
        received = recv(m_socket, reinterpret_cast<char*>(data.data() + totalReceived),
                       static_cast<int>(packetSize - totalReceived), 0);
        if (received <= 0) {
            response.error = "Connection lost during receive";
            return response;
        }
        totalReceived += received;
    }

    // Parse
    memcpy(&response.requestId, data.data(), 4);
    memcpy(&response.type, data.data() + 4, 4);

    if (packetSize > 10) {
        response.body = std::string(reinterpret_cast<char*>(data.data() + 8), packetSize - 10);
    }

    response.success = true;
    return response;
}

bool RconClient::testConnection(const std::string& host, int port, const std::string& password) {
    RconClient testClient;
    bool result = testClient.connect(host, port, password);
    testClient.disconnect();
    return result;
}

} // namespace PSM
