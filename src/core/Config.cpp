#include "core/Config.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>

namespace PSM {

Config::Config() {}
Config::~Config() {}

bool Config::load(const std::string& path) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_filePath = path;

    if (!std::filesystem::exists(path)) {
        loadDefaults();
        return false;
    }

    std::ifstream file(path);
    if (!file.is_open()) return false;

    try {
        file >> m_data;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Config parse error: " << e.what() << std::endl;
        return false;
    }
}

bool Config::save() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_filePath.empty()) return false;

    std::filesystem::create_directories(
        std::filesystem::path(m_filePath).parent_path()
    );

    std::ofstream file(m_filePath);
    if (!file.is_open()) return false;

    file << m_data.dump(4);
    return true;
}

void Config::loadDefaults() {
    m_data = {
        {"window", {
            {"width", 1600},
            {"height", 900},
            {"maximized", false}
        }},
        {"ui", {
            {"fontSize", 16.0f},
            {"theme", "dark"}
        }},
        {"vanilla", {
            {"host", "127.0.0.1"},
            {"port", 8212},
            {"password", ""}
        }},
        {"paldefender", {
            {"host", "127.0.0.1"},
            {"port", 8214},
            {"apiKey", ""}
        }},
        {"rcon", {
            {"host", "127.0.0.1"},
            {"port", 25575},
            {"password", ""}
        }},
        {"pst", {
            {"levelSavPath", ""},
            {"syncInterval", 60}
        }},
        {"server", {
            {"exePath", ""},
            {"workingDir", ""},
            {"launchArgs", nlohmann::json::array()},
            {"autoLaunchOnStart", false},
            {"autoRestartOnCrash", false},
            {"restartDelay", 10},
            {"maxRestartAttempts", 5},
            {"pollInterval", 5},
            {"gracefulShutdown", true},
            {"shutdownCountdown", 30},
            {"shutdownMessage", "Server shutting down in {seconds} seconds!"}
        }},
        {"backup", {
            {"outputPath", "backups"},
            {"intervalMinutes", 60},
            {"retentionDays", 7}
        }},
        {"database", {
            {"path", "palworld_manager.db"}
        }},
        {"notifications", {
            {"desktopOnCrash", true},
            {"desktopOnBan", true},
            {"desktopOnCheat", true},
            {"soundOnCrash", true}
        }},
        {"scan", {
            {"intervalMinutes", 30},
            {"autoAction", false},
            {"suspicionThreshold", 50}
        }},
        {"api", {
            {"enabled", false},
            {"port", 8300}
        }},
        {"app", {
            {"threadPoolSize", 4}
        }}
    };
}

nlohmann::json Config::getNestedValue(const std::string& key) const {
    std::istringstream stream(key);
    std::string token;
    const nlohmann::json* current = &m_data;

    while (std::getline(stream, token, '.')) {
        if (current->contains(token)) {
            current = &(*current)[token];
        } else {
            return nlohmann::json();
        }
    }
    return *current;
}

void Config::setNestedValue(const std::string& key, const nlohmann::json& value) {
    std::istringstream stream(key);
    std::string token;
    std::vector<std::string> tokens;

    while (std::getline(stream, token, '.')) {
        tokens.push_back(token);
    }

    nlohmann::json* current = &m_data;
    for (size_t i = 0; i < tokens.size() - 1; ++i) {
        if (!current->contains(tokens[i])) {
            (*current)[tokens[i]] = nlohmann::json::object();
        }
        current = &(*current)[tokens[i]];
    }
    (*current)[tokens.back()] = value;
}

std::string Config::getString(const std::string& key, const std::string& defaultVal) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto val = getNestedValue(key);
    if (val.is_string()) return val.get<std::string>();
    return defaultVal;
}

int Config::getInt(const std::string& key, int defaultVal) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto val = getNestedValue(key);
    if (val.is_number_integer()) return val.get<int>();
    return defaultVal;
}

float Config::getFloat(const std::string& key, float defaultVal) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto val = getNestedValue(key);
    if (val.is_number()) return val.get<float>();
    return defaultVal;
}

bool Config::getBool(const std::string& key, bool defaultVal) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto val = getNestedValue(key);
    if (val.is_boolean()) return val.get<bool>();
    return defaultVal;
}

void Config::setString(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(m_mutex);
    setNestedValue(key, value);
}

void Config::setInt(const std::string& key, int value) {
    std::lock_guard<std::mutex> lock(m_mutex);
    setNestedValue(key, value);
}

void Config::setFloat(const std::string& key, float value) {
    std::lock_guard<std::mutex> lock(m_mutex);
    setNestedValue(key, value);
}

void Config::setBool(const std::string& key, bool value) {
    std::lock_guard<std::mutex> lock(m_mutex);
    setNestedValue(key, value);
}

nlohmann::json& Config::data() { return m_data; }
const nlohmann::json& Config::data() const { return m_data; }

} // namespace PSM