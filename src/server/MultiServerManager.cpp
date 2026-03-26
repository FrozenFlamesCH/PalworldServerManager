#include "server/MultiServerManager.h"
#include <algorithm>

namespace PSM {

MultiServerManager::MultiServerManager() {}
MultiServerManager::~MultiServerManager() {}

void MultiServerManager::addServer(const ServerProfile& profile) {
    // Check for duplicate ID
    for (auto& s : m_servers) {
        if (s.id == profile.id) {
            s = profile; // Update existing
            return;
        }
    }
    m_servers.push_back(profile);

    if (m_servers.size() == 1) {
        setActiveServer(profile.id);
    }
}

void MultiServerManager::removeServer(const std::string& id) {
    m_servers.erase(
        std::remove_if(m_servers.begin(), m_servers.end(),
            [&id](const ServerProfile& s) { return s.id == id; }),
        m_servers.end());

    if (m_activeServerId == id) {
        m_activeServerId = m_servers.empty() ? "" : m_servers[0].id;
    }
}

void MultiServerManager::setActiveServer(const std::string& id) {
    for (auto& s : m_servers) {
        s.isActive = (s.id == id);
    }
    m_activeServerId = id;
}

ServerProfile* MultiServerManager::getActiveServer() {
    for (auto& s : m_servers) {
        if (s.id == m_activeServerId) return &s;
    }
    return nullptr;
}

const std::vector<ServerProfile>& MultiServerManager::getAllServers() const {
    return m_servers;
}

ServerProfile* MultiServerManager::getServer(const std::string& id) {
    for (auto& s : m_servers) {
        if (s.id == id) return &s;
    }
    return nullptr;
}

bool MultiServerManager::switchTo(const std::string& id) {
    for (auto& s : m_servers) {
        if (s.id == id) {
            setActiveServer(id);
            return true;
        }
    }
    return false;
}

std::string MultiServerManager::activeServerId() const {
    return m_activeServerId;
}

void MultiServerManager::loadFromJson(const nlohmann::json& data) {
    m_servers.clear();
    if (!data.is_array()) return;

    for (auto& entry : data) {
        ServerProfile profile;
        profile.id = entry.value("id", "");
        profile.name = entry.value("name", "");
        profile.vanillaHost = entry.value("vanillaHost", "127.0.0.1");
        profile.vanillaPort = entry.value("vanillaPort", 8212);
        profile.vanillaPassword = entry.value("vanillaPassword", "");
        profile.pdHost = entry.value("pdHost", "127.0.0.1");
        profile.pdPort = entry.value("pdPort", 8214);
        profile.pdApiKey = entry.value("pdApiKey", "");
        profile.rconHost = entry.value("rconHost", "127.0.0.1");
        profile.rconPort = entry.value("rconPort", 25575);
        profile.rconPassword = entry.value("rconPassword", "");
        profile.exePath = entry.value("exePath", "");
        profile.savePath = entry.value("savePath", "");
        m_servers.push_back(profile);
    }

    m_activeServerId = m_servers.empty() ? "" : m_servers[0].id;
}

nlohmann::json MultiServerManager::toJson() const {
    nlohmann::json arr = nlohmann::json::array();
    for (auto& s : m_servers) {
        arr.push_back({
            {"id", s.id},
            {"name", s.name},
            {"vanillaHost", s.vanillaHost},
            {"vanillaPort", s.vanillaPort},
            {"vanillaPassword", s.vanillaPassword},
            {"pdHost", s.pdHost},
            {"pdPort", s.pdPort},
            {"pdApiKey", s.pdApiKey},
            {"rconHost", s.rconHost},
            {"rconPort", s.rconPort},
            {"rconPassword", s.rconPassword},
            {"exePath", s.exePath},
            {"savePath", s.savePath}
        });
    }
    return arr;
}

} // namespace PSM
