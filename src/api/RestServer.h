#pragma once

#include <string>
#include <thread>
#include <atomic>
#include <functional>
#include <map>

class Application;

namespace PSM {

class RestServer {
public:
    RestServer(int port, Application& app);
    ~RestServer();

    void start();
    void stop();
    bool isRunning() const;

private:
    void serverThread();
    std::string handleRequest(const std::string& method, const std::string& path,
                             const std::string& body);

    int m_port;
    Application& m_app;
    std::atomic<bool> m_running{false};
    std::thread m_thread;
};

} // namespace PSM
