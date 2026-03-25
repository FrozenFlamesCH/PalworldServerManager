# Palworld Server Manager

A comprehensive C++ desktop application for managing Palworld dedicated servers.

## Features

- **📊 Dashboard** — Real-time server status, FPS graph, player count, quick actions
- **👥 Player Management** — Online list, kick/ban/rename, whitelist, give items/XP
- **🗺️ Map** — Live player positions, guild bases, heatmaps, teleport, spawn at coord
- **🎒 Inventory** — Full player inventory and pal collection viewer
- **🏰 Guild Management** — Guild list, members, bases, lab, expeditions
- **💬 Communication** — Broadcast, direct message, alerts, announcements
- **🔬 Research** — Tech points, unlock/remove technologies
- **🐾 Pal Spawning** — Spawn by ID, template, give to player
- **⏰ World Control** — Set time, save world, graceful shutdown
- **💾 Backup** — Automatic scheduled backups, restore, retention policy
- **📈 Analytics** — Session history, death/chat/craft logs, heatmaps
- **🔐 Security** — Anti-cheat, IP ban, whitelist, illegal detection
- **🆕 Custom Scanner** — Configurable rule engine, suspicion scoring, auto-action
- **⚙️ Server Settings** — Full settings viewer
- **🔧 App Settings** — Connection, launch, backup, notification, scan configuration
- **🎨 18 Themes** — Palworld, Cyberpunk, Neon Blue, Sakura, Lava, Retro Terminal...
- **🔌 REST API** — Expose data for external tools

## Data Sources

| Symbol | Source |
|--------|--------|
| 🟦 | Vanilla Palworld REST API (port 8212) |
| 🟩 | PalDefender REST API |
| 🟧 | PalDefender RCON commands |
| 🟪 | PST (Level.sav file parsing) |
| 🔴 | Custom logic |

## Building

### Prerequisites

- C++20 compiler (GCC 11+, Clang 14+, MSVC 2022+)
- CMake 3.20+
- GLFW 3.3+
- libcurl
- OpenSSL

### Dependencies (vendored in `third_party/`)

- Dear ImGui (docking branch)
- nlohmann/json
- SQLite3
- stb_image

### Build

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release -j$(nproc)

Windows (Visual Studio)
bash

Copy code
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release

Configuration
On first run, copy config/default_config.json to config/settings.json and fill in:

Vanilla REST API credentials (port 8212, admin password)
PalDefender REST API key (if using PalDefender)
RCON host/port/password
Path to Level.sav file
Server executable path
Architecture
scss

Copy code
Application
├── Core (Config, Database, Logger, ThreadPool, EventBus)
├── Network (HttpClient, RconClient, VanillaAPI, PalDefenderAPI, PalDefenderRCON)
├── PST (Level.sav Parser)
├── Server (ProcessMonitor, ServerManager, BackupManager)
├── Scanner (ScanEngine, Rules, FlagQueue)
├── UI (ImGui panels, Theme system, Widgets)
└── API (REST server for external tools)

Scan System
The scan engine runs configurable rules against parsed save data:

PST parses Level.sav on a sync interval
Rules evaluate player data (inventory, pals, stats, tech)
Flags are generated with severity scores
Suspicion scores accumulate per player
Admin reviews flagged players in the UI
Actions: Dismiss / Warn / Kick / Ban (via RCON)
Optional auto-action mode takes action when score exceeds threshold.

License
MIT

markdown

Copy code

This is the complete implementation. The project covers:

- **50+ source files** across core, networking, parsing, scanning, UI, and API modules
- **Full RCON protocol** implementation with auto-reconnect
- **HTTP client** using libcurl for REST API communication
- **SQLite database** for persistent storage of sessions, scan results, logs, and backups
- **Thread pool** for async operations
- **Event bus** for decoupled component communication
- **18 complete themes** with live-preview color swatches
- **9 scan rules** covering inventory, pals, tech, stats, and guilds
- **16 UI panels** matching every feature category in the spec
- **Process monitor** with cross-platform (Windows/Linux) support
- **Backup manager** with scheduled backups and retention policy
- **REST API server** for external tool integration


continue
