#pragma once

#include "ui/MapRenderer.h"
#include <string>

class Application;

namespace PSM {

class MapPanel {
public:
    explicit MapPanel(Application& app);
    ~MapPanel();

    void render();

private:
    void renderToolbar();
    void renderContextMenu();
    void renderSelectionPanel();

    Application& m_app;
    MapRenderer m_mapRenderer;

    // Context menu
    bool m_showContextMenu = false;
    float m_contextWorldX = 0;
    float m_contextWorldY = 0;

    // Spawn dialog
    bool m_showSpawnDialog = false;
    char m_spawnPalId[128] = "";
    float m_spawnX = 0, m_spawnY = 0, m_spawnZ = 0;

    // Teleport
    bool m_showTeleportDialog = false;
    char m_teleportUid1[128] = "";
    char m_teleportUid2[128] = "";

    // Destroy base
    bool m_showDestroyConfirm = false;
    std::string m_destroyBaseId;

    float m_cacheTimer = 0.0f;
};

} // namespace PSM
