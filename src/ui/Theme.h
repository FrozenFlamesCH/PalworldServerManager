#pragma once

#include <string>
#include <vector>
#include <imgui.h>

namespace PSM {

struct ThemeColors {
    ImVec4 windowBg;
    ImVec4 childBg;
    ImVec4 popupBg;
    ImVec4 border;
    ImVec4 frameBg;
    ImVec4 frameBgHovered;
    ImVec4 frameBgActive;
    ImVec4 titleBg;
    ImVec4 titleBgActive;
    ImVec4 menuBarBg;
    ImVec4 scrollbarBg;
    ImVec4 scrollbarGrab;
    ImVec4 checkMark;
    ImVec4 sliderGrab;
    ImVec4 button;
    ImVec4 buttonHovered;
    ImVec4 buttonActive;
    ImVec4 header;
    ImVec4 headerHovered;
    ImVec4 headerActive;
    ImVec4 separator;
    ImVec4 tab;
    ImVec4 tabHovered;
    ImVec4 tabActive;
    ImVec4 text;
    ImVec4 textDisabled;
    ImVec4 plotLines;
    ImVec4 plotHistogram;
    ImVec4 tableHeaderBg;
    ImVec4 tableRowBg;
    ImVec4 tableRowBgAlt;
    // Custom colors
    ImVec4 success;
    ImVec4 warning;
    ImVec4 error;
    ImVec4 info;
    ImVec4 accent;
};

struct ThemeDefinition {
    std::string id;
    std::string name;
    std::string emoji;
    std::string description;
    ThemeColors colors;
    float windowRounding = 6.0f;
    float frameRounding = 4.0f;
    float grabRounding = 3.0f;
    float tabRounding = 4.0f;
};

class ThemeManager {
public:
    ThemeManager();
    ~ThemeManager();

    void initialize();
    void apply(const std::string& themeId);
    void applyToImGui(const ThemeDefinition& theme);

    const std::vector<ThemeDefinition>& getThemes() const;
    const ThemeDefinition& getCurrentTheme() const;
    std::string getCurrentThemeId() const;

    // Custom theme colors for UI elements
    const ThemeColors& colors() const;

private:
    void registerThemes();

    std::vector<ThemeDefinition> m_themes;
    int m_currentThemeIndex = 0;
};

} // namespace PSM
