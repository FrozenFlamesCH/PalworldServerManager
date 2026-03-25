#pragma once

#include <string>

class Application;

namespace PSM {

class InventoryPanel {
public:
    explicit InventoryPanel(Application& app);
    ~InventoryPanel();

    void render();

private:
    void renderPlayerSelector();
    void renderInventoryTab();
    void renderPalTab();
    void renderGiveItemPopup();
    void renderGivePalPopup();

    Application& m_app;

    std::string m_selectedUid;
    std::string m_selectedName;

    bool m_showGiveItem = false;
    bool m_showGivePal = false;

    char m_itemId[128] = "";
    int m_itemCount = 1;
    char m_palId[128] = "";
    int m_palLevel = 1;
    char m_searchBuf[128] = "";
};

} // namespace PSM
