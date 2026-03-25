#include "server/ProcessMonitor.h"
#include <chrono>
#include <iostream>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#include <tlhelp32.h>
#else
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#endif

namespace PSM {

ProcessMonitor::ProcessMonitor(const std::string& exePath, int pollInterval)
    : m_exePath(exePath), m_pollInterval(pollInterval) {}

ProcessMonitor::~ProcessMonitor() {
    stop();
}

void ProcessMonitor::setExePath(const std::string& path) {
    m_exePath = path;
}

void ProcessMonitor::setPollInterval(int seconds) {
    m_pollInterval = seconds;
}

void ProcessMonitor::start() {
    if (m_monitoring) return;
    m_monitoring = true;
    m_monitorThread = std::thread(&ProcessMonitor::monitorThread, this);
}

void ProcessMonitor::stop() {
    m_monitoring = false;
    if (m_monitorThread.joinable()) {
        m_monitorThread.join();
    }
}

ServerProcessState ProcessMonitor::getState() const {
    return m_state;
}

std::string ProcessMonitor::getStateString() const {
    switch (m_state) {
        case ServerProcessState::Stopped: return "Stopped";
        case ServerProcessState::Starting: return "Starting";
        case ServerProcessState::Running: return "Running";
        case ServerProcessState::Crashed: return "Crashed";
        case ServerProcessState::Stopping: return "Stopping";
    }
    return "Unknown";
}

bool ProcessMonitor::startProcess(const std::string& workingDir,
                                   const std::vector<std::string>& args) {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (m_state == ServerProcessState::Running) return false;

    auto oldState = m_state.load();
    m_state = ServerProcessState::Starting;

    for (auto& cb : m_stateCallbacks) cb(oldState, ServerProcessState::Starting);

#ifdef _WIN32
    STARTUPINFOA si{};
    PROCESS_INFORMATION pi{};
    si.cb = sizeof(si);

    std::string cmdLine = "\"" + m_exePath + "\"";
    for (auto& arg : args) {
        cmdLine += " " + arg;
    }

    std::string wd = workingDir.empty() ?
        std::filesystem::path(m_exePath).parent_path().string() : workingDir;

    if (CreateProcessA(nullptr, cmdLine.data(), nullptr, nullptr, FALSE,
                       CREATE_NEW_PROCESS_GROUP, nullptr,
                       wd.empty() ? nullptr : wd.c_str(), &si, &pi)) {
        m_processHandle = pi.hProcess;
        m_threadHandle = pi.hThread;
        m_processId = pi.dwProcessId;

        m_startTime = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();

        m_state = ServerProcessState::Running;
        for (auto& cb : m_stateCallbacks) cb(ServerProcessState::Starting, ServerProcessState::Running);
        return true;
    }
#else
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        std::string wd = workingDir.empty() ?
            std::filesystem::path(m_exePath).parent_path().string() : workingDir;
        if (!wd.empty()) chdir(wd.c_str());

        std::vector<char*> argv;
        argv.push_back(const_cast<char*>(m_exePath.c_str()));
        for (auto& arg : args) {
            argv.push_back(const_cast<char*>(arg.c_str()));
        }
        argv.push_back(nullptr);

        execv(m_exePath.c_str(), argv.data());
        _exit(1);
    } else if (pid > 0) {
        m_processId = pid;
        m_startTime = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        m_state = ServerProcessState::Running;
        for (auto& cb : m_stateCallbacks) cb(ServerProcessState::Starting, ServerProcessState::Running);
        return true;
    }
#endif

    m_state = ServerProcessState::Stopped;
    return false;
}

bool ProcessMonitor::stopProcess() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_state != ServerProcessState::Running) return false;

    auto oldState = m_state.load();
    m_state = ServerProcessState::Stopping;
    for (auto& cb : m_stateCallbacks) cb(oldState, ServerProcessState::Stopping);

    // The actual graceful stop should be done via API/RCON first
    // This is just the process-level termination
    return true;
}

bool ProcessMonitor::killProcess() {
    std::lock_guard<std::mutex> lock(m_mutex);

#ifdef _WIN32
    if (m_processHandle) {
        TerminateProcess(m_processHandle, 1);
        CloseHandle(m_processHandle);
        CloseHandle(m_threadHandle);
        m_processHandle = nullptr;
        m_threadHandle = nullptr;
    }
#else
    if (m_processId > 0) {
        kill(m_processId, SIGKILL);
        m_processId = 0;
    }
#endif

    auto oldState = m_state.load();
    m_state = ServerProcessState::Stopped;
    for (auto& cb : m_stateCallbacks) cb(oldState, ServerProcessState::Stopped);
    return true;
}

bool ProcessMonitor::isProcessRunning() const {
    return m_state == ServerProcessState::Running;
}

bool ProcessMonitor::checkProcess() {
#ifdef _WIN32
    if (!m_processHandle) return false;
    DWORD exitCode;
    if (GetExitCodeProcess(m_processHandle, &exitCode)) {
        return exitCode == STILL_ACTIVE;
    }
    return false;
#else
    if (m_processId <= 0) return false;
    int status;
    pid_t result = waitpid(m_processId, &status, WNOHANG);
    return result == 0; // Still running
#endif
}

void ProcessMonitor::monitorThread() {
    while (m_monitoring) {
        if (m_state == ServerProcessState::Running || m_state == ServerProcessState::Stopping) {
            if (!checkProcess()) {
                auto oldState = m_state.load();
                if (oldState == ServerProcessState::Stopping) {
                    m_state = ServerProcessState::Stopped;
                } else {
                    m_state = ServerProcessState::Crashed;
                }
                for (auto& cb : m_stateCallbacks) cb(oldState, m_state.load());

#ifdef _WIN32
                if (m_processHandle) {
                    CloseHandle(m_processHandle);
                    CloseHandle(m_threadHandle);
                    m_processHandle = nullptr;
                    m_threadHandle = nullptr;
                }
#else
                m_processId = 0;
#endif
            }
        }

        for (int i = 0; i < m_pollInterval * 10 && m_monitoring; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}

void ProcessMonitor::onStateChange(StateChangeCallback callback) {
    m_stateCallbacks.push_back(std::move(callback));
}

int64_t ProcessMonitor::getProcessUptime() const {
    if (m_state != ServerProcessState::Running) return 0;
    auto now = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    return now - m_startTime;
}

} // namespace PSM
