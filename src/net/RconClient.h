#pragma once

#include <string>
#include <cstdint>
#include <mutex>
#include <vector>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
typedef SOCKET SocketType;
#define INVALID_SOCK INVALID_SOCKET
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
typedef int SocketType;
#define INVALID_SOCK -1
#endif

namespace PSM {

struct RconResponse {
    int32_t requestId;
    int32_t type;
    std::string body;
    bool success = false;
    std::string error;
};

class RconClient {
public:
    RconClient();
    ~RconClient();

    bool connect(const std::string& host, int port, const std::string& password);
    void disconnect();
    bool isConnected() const;

    RconResponse sendCommand(const std::string& command);
    bool authenticate(const std::string& password);

    // Test connection
    bool testConnection(const std::string& host, int port, const std::string& password);

private:
    bool sendPacket(int32_t requestId, int32_t type, const std::string& body);
    RconResponse receivePacket();

    static constexpr int32_t SERVERDATA_AUTH = 3;
    static constexpr int32_t SERVERDATA_AUTH_RESPONSE = 2;
    static constexpr int32_t SERVERDATA_EXECCOMMAND = 2;
    static constexpr int32_t SERVERDATA_RESPONSE_VALUE = 0;

    SocketType m_socket = INVALID_SOCK;
    bool m_connected = false;
    bool m_authenticated = false;
    int32_t m_requestId = 0;
    std::mutex m_mutex;
    std::string m_host;
    int m_port = 0;
    std::string m_password;
};

} // namespace PSM
