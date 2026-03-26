#pragma once

#include <string>
#include <mutex>
#include <nlohmann/json.hpp>

namespace PSM {

class Config {
public:
    Config();
    ~Config();

    bool load(const std::string& path);
    bool save();
    void loadDefaults();

    std::string getString(const std::string& key, const std::string& defaultVal = "") const;
    int getInt(const std::string& key, int defaultVal = 0) const;
    float getFloat(const std::string& key, float defaultVal = 0.0f) const;
    bool getBool(const std::string& key, bool defaultVal = false) const;

    void setString(const std::string& key, const std::string& value);
    void setInt(const std::string& key, int value);
    void setFloat(const std::string& key, float value);
    void setBool(const std::string& key, bool value);

    nlohmann::json& data();
    const nlohmann::json& data() const;

private:
    nlohmann::json getNestedValue(const std::string& key) const;
    void setNestedValue(const std::string& key, const nlohmann::json& value);

    mutable std::mutex m_mutex;
    nlohmann::json m_data;
    std::string m_filePath;
};

} // namespace PSM