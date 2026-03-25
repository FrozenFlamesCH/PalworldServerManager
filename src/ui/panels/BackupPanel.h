// BackupPanel.h
#pragma once
class Application;
namespace PSM {
class BackupPanel {
public:
    explicit BackupPanel(Application& app) : m_app(app) {}
    void render();
private:
    Application& m_app;
    bool m_restoreConfirm = false;
    std::string m_restorePath;
};
} // namespace PSM

cpp
