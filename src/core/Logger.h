#pragma once

#include <string>
#include <fstream>
#include <mutex>
#include <vector>
#include <functional>
#include <chrono>
#include <fmt/format.h>

namespace PSM {

enum class LogLevel {
    Trace,
    Debug,
    Info,
    Warn,
    Error,
    Fatal
};

struct LogEntry {
    LogLevel level;
    std::string message;
    std::chrono::system_clock::time_point timestamp;
    std::string category;
};

class Logger {
public:
    Logger();
    ~Logger();

    bool initialize(const std::string& logFilePath);

    template<typename... Args>
    void trace(fmt::format_string<Args...> fmt, Args&&... args) {
        log(LogLevel::Trace, fmt::format(fmt, std::forward<Args>(args)...));
    }

    template<typename... Args>
    void debug(fmt::format_string<Args...> fmt, Args&&... args) {
        log(LogLevel::Debug, fmt::format(fmt, std::forward<Args>(args)...));
    }

    template<typename... Args>
    void info(fmt::format_string<Args...> fmt, Args&&... args) {
        log(LogLevel::Info, fmt::format(fmt, std::forward<Args>(args)...));
    }

    template<typename... Args>
    void warn(fmt::format_string<Args...> fmt, Args&&... args) {
        log(LogLevel::Warn, fmt::format(fmt, std::forward<Args>(args)...));
    }

    template<typename... Args>
    void error(fmt::format_string<Args...> fmt, Args&&... args) {
        log(LogLevel::Error, fmt::format(fmt, std::forward<Args>(args)...));
    }

    template<typename... Args>
    void fatal(fmt::format_string<Args...> fmt, Args&&... args) {
        log(LogLevel::Fatal, fmt::format(fmt, std::forward<Args>(args)...));
    }

    // Simple string overloads
    void info(const std::string& msg) { log(LogLevel::Info, msg); }
    void warn(const std::string& msg) { log(LogLevel::Warn, msg); }
    void error(const std::string& msg) { log(LogLevel::Error, msg); }

    void log(LogLevel level, const std::string& message, const std::string& category = "");

    const std::vector<LogEntry>& getRecentEntries() const;
    void setLogLevel(LogLevel level);
    void addCallback(std::function<void(const LogEntry&)> callback);

private:
    std::string levelToString(LogLevel level) const;

    mutable std::mutex m_mutex;
    std::ofstream m_file;
    LogLevel m_minLevel = LogLevel::Info;
    std::vector<LogEntry> m_recentEntries;
    std::vector<std::function<void(const LogEntry&)>> m_callbacks;
    size_t m_maxRecentEntries = 1000;
};

} // namespace PSM