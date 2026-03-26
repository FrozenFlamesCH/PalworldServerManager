#pragma once

#include <string>
#include <vector>
#include "models/Player.h"

class Application;

namespace PSM {

class PlayerPanel {
public:
    explicit PlayerPanel(Application& app);
    ~PlayerPanel();

    void render();

private:
    void renderToolbar();
    void renderOnlineTab();
    void renderAllPlayersTab();
    void renderBannedTab();
    void renderPlayerDetail(const Player& player);
    void renderPlayerActions(const Player& player);
    void renderKickDialog();
    void renderBanDialog();
    void renderRenameDialog();
    void renderGiveExpDialog();
    void renderGiveItemDialog();
    void renderWhitelistDialog();

    Application& m_app;

    char m_searchBuf[256] = "";
    int m_selectedTab = 0;

    // Selected player for detail view
    std::string m_selectedPlayerUid;
    bool m_showPlayerDetail = false;

    // Dialog states
    bool m_showKickDialog = false;
    bool m_showBanDialog = false;
    bool m_showRenameDialog = false;
    bool m_showGiveExpDialog = false;
    bool m_showGiveItemDialog = false;
    bool m_showWhitelistDialog = false;

    // Dialog data
    std::string m_targetUid;
    std::string m_targetName;
    char m_kickReason[256] = "";
    char m_banReason[256] = "";
    char m_newName[128] = "";
    int m_giveExpAmount = 0;
    char m_giveItemId[128] = "";
    int m_giveItemCount = 1;
    int m_giveTechPoints = 0;
    int m_giveAncientPoints = 0;
    int m_giveEffigies = 0;
    int m_giveStatusPoints = 0;

    // Cached data
    std::vector<Player> m_cachedOnline;
    std::vector<Player> m_cachedAll;
    float m_cacheTimer = 0.0f;
};

} // namespace PSM
