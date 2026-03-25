#pragma once

#include <string>
#include <cstdint>

namespace PSM {

struct GameLogEntry {
    int id = 0;
    std::string category;
    std::string message;
    std::string playerUid;
    int64_t timestamp = 0;
};

// Log categories
namespace LogCategory {
    constexpr const char* DEATH = "death";
    constexpr const char* CHEAT = "cheat";
    constexpr const char* RCON = "rcon";
    constexpr const char* CHAT = "chat";
    constexpr const char* BUILD = "building";
    constexpr const char* CAPTURE = "capture";
    constexpr const char* CRAFT = "craft";
    constexpr const char* TECH = "tech";
    constexpr const char* OIL_RIG = "oil_rig";
    constexpr const char* HELICOPTER = "helicopter";
    constexpr const char* SESSION = "session";
    constexpr const char* SCAN = "scan";
    constexpr const char* BACKUP = "backup";
}

} // namespace PSM
