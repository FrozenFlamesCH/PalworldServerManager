#include "api/RestServer.h"
#include "Application.h"
#include "core/Logger.h"
#include "server/ServerManager.h"
#include "scanner/ScanEngine.h"
#include "pst/LevelSavParser.h"
#include "core/Database.h"

#include <nlohmann/json.hpp>
#include <sstream>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#endif

namespace PSM {

RestServer::RestServer(int port, Application& app)
    : m_port(port), m_app(app) {}

RestServer::~RestServer() {
    stop();
}

void RestServer::start() {
    if (m_running) return;
    m_running = true;
    m_thread = std::thread(&RestServer::serverThread, this);
    m_app.logger().info("REST API server started on port " + std::to_string(m_port));
}

void RestServer::stop() {
    m_running = false;
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

bool RestServer::isRunning() const {
    return m_running;
}

void RestServer::serverThread() {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

    int serverFd = static_cast<int>(socket(AF_INET, SOCK_STREAM, 0));
    if (serverFd < 0) {
        m_app.logger().error("REST server: Failed to create socket");
        return;
    }

    int opt = 1;
    setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&opt), sizeof(opt));

    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(static_cast<uint16_t>(m_port));

    if (bind(serverFd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
        m_app.logger().error("REST server: Failed to bind");
#ifdef _WIN32
        closesocket(serverFd);
#else
        close(serverFd);
#endif
        return;
    }

    listen(serverFd, 10);

    // Set non-blocking timeout
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    setsockopt(serverFd, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&timeout), sizeof(timeout));

    while (m_running) {
        struct sockaddr_in clientAddr{};
        socklen_t clientLen = sizeof(clientAddr);
        int clientFd = static_cast<int>(accept(serverFd,
            reinterpret_cast<struct sockaddr*>(&clientAddr), &clientLen));

        if (clientFd < 0) continue;

        // Read request
        char buffer[4096] = {};
        recv(clientFd, buffer, sizeof(buffer) - 1, 0);

        // Parse HTTP request (simplified)
        std::string request(buffer);
        std::string method, path, body;

        std::istringstream reqStream(request);
        reqStream >> method >> path;

        // Find body (after double CRLF)
        size_t bodyStart = request.find("\r\n\r\n");
        if (bodyStart != std::string::npos) {
            body = request.substr(bodyStart + 4);
        }

        // Handle and respond
        std::string responseBody = handleRequest(method, path, body);

        std::string httpResponse =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/json\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Content-Length: " + std::to_string(responseBody.size()) + "\r\n"
            "\r\n" + responseBody;

        send(clientFd, httpResponse.c_str(), static_cast<int>(httpResponse.size()), 0);

#ifdef _WIN32
        closesocket(clientFd);
#else
        close(clientFd);
#endif
    }

#ifdef _WIN32
    closesocket(serverFd);
    WSACleanup();
#else
    close(serverFd);
#endif
}

std::string RestServer::handleRequest(const std::string& method, const std::string& path,
                                       const std::string& body) {
    nlohmann::json response;

    if (path == "/api/status") {
        auto info = m_app.serverManager().cachedInfo();
        response = info.toJson();
    }
    else if (path == "/api/players") {
        auto players = m_app.serverManager().cachedPlayers();
        response = nlohmann::json::array();
        for (auto& p : players) {
            response.push_back(p.toJson());
        }
    }
    else if (path == "/api/players/all") {
        auto players = m_app.levelSavParser().getAllPlayers();
        response = nlohmann::json::array();
        for (auto& p : players) {
            response.push_back(p.toJson());
        }
    }
    else if (path == "/api/scan/reports") {
        auto reports = m_app.scanEngine().getReports();
        response = nlohmann::json::array();
        for (auto& r : reports) {
            response.push_back({
                {"uid", r.playerUid},
                {"name", r.playerName},
                {"score", r.totalScore},
                {"flagCount", r.flags.size()}
            });
        }
    }
    else if (path == "/api/logs") {
        response = m_app.database().getLogs("", 100);
    }
    else if (path == "/api/backups") {
        response = m_app.database().getBackupHistory(50);
    }
    else {
        response = {{"error", "Unknown endpoint"}, {"path", path}};
    }

    return response.dump(2);
}

} // namespace PSM
