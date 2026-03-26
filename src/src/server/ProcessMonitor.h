#pragma once

#include <string>
#include <atomic>
#include <thread>
#include <mutex>
#include <functional>

namespace PSM {

enum class ServerProcessState {
    Stopped,
    Starting,
    Running,
    Crashed,
    Stopping
};

class ProcessMonitor {
public:
    ProcessMonitor(const std::string& exePath, int pollInterval);
    ~ProcessMonitor();

    void setExePath(const std::string& path);
    void setPollInterval(int seconds);

    void start();
    void stop();

    ServerProcessState getState() const;
    std::string getStateString() const;

    bool startProcess(const std::string& workingDir = "",
                      const std::vector<std::string>& args = {});
    bool stopProcess();
    bool killProcess();
    bool isProcessRunning() const;

    using StateChangeCallback = std::function<void(ServerProcessState, ServerProcessState)>;
    void onStateChange(StateChangeCallback callback);

    int64_t getProcessUptime() const;

#ifdef _WIN32
    void* getProcessHandle() const;
#else
    int getProcessId() const;
#endif

private:
    void monitorThread();
    bool checkProcess();

    std::string m_exePath;
    int m_pollInterval;
    std::atomic<bool> m_monitoring{false};
    std::thread m_monitorThread;
    mutable std::mutex m_mutex;

    std::atomic<ServerProcessState> m_state{ServerProcessState::Stopped};
    int64_t m_startTime = 0;

#ifdef _WIN32
    void* m_processHandle = nullptr;
    void* m_threadHandle = nullptr;
    unsigned long m_processId = 0;
#else
    pid_t m_processId = 0;
#endif

    std::vector<StateChangeCallback> m_stateCallbacks;
};

} // namespace PSM
