// PalSpawnPanel.h
#pragma once
class Application;
namespace PSM {
class PalSpawnPanel {
public:
    explicit PalSpawnPanel(Application& app) : m_app(app) {}
    void render();
private:
    Application& m_app;
    char m_palId[128] = "";
    char m_targetUid[128] = "";
    char m_templatePath[512] = "";
    char m_summonFile[512] = "";
    int m_level = 1;
    float m_x = 0, m_y = 0, m_z = 0;
};
} // namespace PSM

cpp
