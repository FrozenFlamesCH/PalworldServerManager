#pragma once

#include <string>
#include <vector>
#include "models/Guild.h"

class Application;

namespace PSM {

class GuildPanel {
public:
    explicit GuildPanel(Application& app);
    ~GuildPanel();

    void render();

private:
    void renderGuildList();
    void renderGuildDetail(const Guild& guild);

    Application& m_app;

    std::vector<Guild> m_cachedGuilds;
    float m_cacheTimer = 99.0f; // force initial load
    std::string m_selectedGuildId;
    char m_searchBuf[128] = "";
};

} // namespace PSM
