#pragma once

#include <string>
#include <vector>
#include "models/ScanResult.h"

class Application;

namespace PSM {

class ScanPanel {
public:
    explicit ScanPanel(Application& app);
    ~ScanPanel();

    void render();

private:
    void renderControls();
    void renderRules();
    void renderFlagQueue();
    void renderPlayerReport(const std::string& uid);
    void renderScanHistory();

    Application& m_app;

    std::string m_selectedReportUid;
    bool m_scanRunning = false;
    char m_manualScanUid[128] = "";
};

} // namespace PSM
