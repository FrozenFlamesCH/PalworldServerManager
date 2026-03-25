#include "core/Logger.h"
#include <iostream>
#include <iomanip>
#include <ctime>
#include <filesystem>

namespace PSM {

Logger::Logger() {}

Logger::~Logger() {
    if (m_file.is_open()) {
        m_file.close();
    }
}

bool Logger::initialize(const std::string& logFilePath) {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::filesystem::create_directories(
        std::filesystem::path(logFilePath).parent_path()
    );
    m_file.open(logFilePath, std::ios::app);
    return m_file.is_open() || logFilePath.empty();
}

void Logger::log(LogLevel level, const std::string& message, const std::string& category) {
    if (level < m_minLevel) return;

    std::lock_guard<std::mutex> lock(m_mutex);

    LogEntry entry;
    entry.level = level;
    entry.message = message;
    entry.timestamp = std::chrono::system_clock::now();
    entry.category = category;

    // Format timestamp
    auto time = std::chrono::system_clock::to_time_t(entry.timestamp);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &time);
#else
    localtime_r(&time, &tm);
#endif

    char timeBuf[64];
    std::strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", &tm);

    std::string formatted = std::string(timeBuf) + " [" + levelToString(level) + "] ";
    if (!category.empty()) {
        formatted += "[" + category + "] ";
    }
    formatted += message;

    // Console output
    std::cout << formatted << std::endl;

    // File output
    if (m_file.is_open()) {
        m_file << formatted << std::endl;
        m_file.flush();
    }

    // Store recent
    m_recentEntries.push_back(entry);
    if (m_recentEntries.size() > m_maxRecentEntries) {
        m_recentEntries.erase(m_recentEntries.begin());
    }

    // Notify callbacks
    for (auto& cb : m_callbacks) {
        cb(entry);
    }
}

const std::vector<LogEntry>& Logger::getRecentEntries() const {
    return m_recentEntries;
}

void Logger::setLogLevel(LogLevel level) {
    m_minLevel = level;
}

void Logger::addCallback(std::function<void(const LogEntry&)> callback) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_callbacks.push_back(std::move(callback));
}

std::string Logger::levelToString(LogLevel level) const {
    switch (level) {
        case LogLevel::Trace: return "TRACE";
        case LogLevel::Debug: return "DEBUG";
        case LogLevel::Info:  return "INFO ";
        case LogLevel::Warn:  return "WARN ";
        case LogLevel::Error: return "ERROR";
        case LogLevel::Fatal: return "FATAL";
        default: return "?????";
    }
}

} // namespace PSM