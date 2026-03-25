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

Build Helper Script
bash

Copy code
#!/bin/bash
# build.sh - Build helper for Palworld Server Manager

set -e

BUILD_TYPE="${1:-Release}"
BUILD_DIR="build"

echo "=== Palworld Server Manager Build ==="
echo "Build type: $BUILD_TYPE"
echo ""

# Check dependencies
check_dep() {
    if ! command -v "$1" &> /dev/null; then
        echo "ERROR: $1 not found. Please install $2"
        exit 1
    fi
}

check_dep cmake "CMake 3.20+"
check_dep g++ "GCC 11+ or Clang 14+"

# Setup third-party dependencies if not present
if [ ! -d "third_party/imgui" ]; then
    echo "Downloading Dear ImGui..."
    git clone --branch docking --depth 1 https://github.com/ocornut/imgui.git third_party/imgui
fi

if [ ! -d "third_party/json" ]; then
    echo "Downloading nlohmann/json..."
    mkdir -p third_party/json/include/nlohmann
    curl -L -o third_party/json/include/nlohmann/json.hpp \
        https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp
fi

if [ ! -f "third_party/sqlite3/sqlite3.c" ]; then
    echo "Downloading SQLite3..."
    mkdir -p third_party/sqlite3
    curl -L -o /tmp/sqlite.zip https://sqlite.org/2024/sqlite-amalgamation-3450100.zip
    unzip -o /tmp/sqlite.zip -d /tmp/sqlite_tmp
    cp /tmp/sqlite_tmp/sqlite-amalgamation-*/sqlite3.c third_party/sqlite3/
    cp /tmp/sqlite_tmp/sqlite-amalgamation-*/sqlite3.h third_party/sqlite3/
    rm -rf /tmp/sqlite.zip /tmp/sqlite_tmp
fi

if [ ! -d "third_party/stb" ]; then
    echo "Downloading stb..."
    mkdir -p third_party/stb
    curl -L -o third_party/stb/stb_image.h \
        https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
fi

# Create config directory
mkdir -p config
if [ ! -f "config/settings.json" ]; then
    cp config/default_config.json config/settings.json 2>/dev/null || true
fi

# Build
echo ""
echo "Configuring..."
cmake -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"

echo ""
echo "Building..."
cmake --build "$BUILD_DIR" --config "$BUILD_TYPE" -j$(nproc)

echo ""
echo "=== Build Complete ==="
echo "Binary: $BUILD_DIR/PalworldServerManager"
echo ""
echo "Run with: ./$BUILD_DIR/PalworldServerManager"

This continuation adds:

PlayerData parser — Complete GVAS binary format reader for individual .sav files with FString, Int32, Int64, Float, Bool, GUID, Struct, Array, Map, and Enum property parsing
SuspicionScore calculator — Weighted scoring with exponential time decay, recency boosting, and score breakdown analysis
FlagQueue — Thread-safe review queue with filtering, sorting, batch dismiss, and callback system
MapRenderer — Full interactive map with zoom/pan, world↔screen coordinate conversion, player dots, guild base markers with color coding, pal count badges, shiny/legendary highlights, heatmap layer, hover tooltips, and coordinate display
DashboardPanel — Fully self-contained dashboard with alert banners, stat cards, FPS ring buffer graph, frame time graph, quick action popups, and live recent activity feed
Advanced scan rules — Shiny accumulation anomaly, inventory-playtime cross-reference, tower boss capture detection
NotificationManager — In-app toast notifications with fade/slide animation, desktop notifications (Windows/Linux/macOS), and sound alerts
MultiServerManager — Multi-server profile management with JSON serialization
Build script — Automated dependency download and build for Linux/macOS

did you finish the source


No, there are still remaining files. Let me complete every remaining source file.
