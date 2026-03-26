#include "ui/Theme.h"
#include <algorithm>

namespace PSM {

ThemeManager::ThemeManager() {}
ThemeManager::~ThemeManager() {}

void ThemeManager::initialize() {
    registerThemes();
    apply("dark"); // Default
}

void ThemeManager::registerThemes() {
    // 🌿 Palworld Theme
    {
        ThemeDefinition t;
        t.id = "palworld";
        t.name = "Palworld";
        t.emoji = "🌿";
        t.description = "Warm earthy greens and ambers inspired by the game";
        t.windowRounding = 8.0f;
        t.frameRounding = 5.0f;

        auto& c = t.colors;
        c.windowBg = ImVec4(0.13f, 0.16f, 0.10f, 1.0f);
        c.childBg = ImVec4(0.15f, 0.19f, 0.12f, 1.0f);
        c.popupBg = ImVec4(0.14f, 0.17f, 0.11f, 0.95f);
        c.border = ImVec4(0.35f, 0.42f, 0.25f, 0.5f);
        c.frameBg = ImVec4(0.20f, 0.25f, 0.15f, 1.0f);
        c.frameBgHovered = ImVec4(0.25f, 0.32f, 0.18f, 1.0f);
        c.frameBgActive = ImVec4(0.30f, 0.38f, 0.22f, 1.0f);
        c.titleBg = ImVec4(0.10f, 0.12f, 0.08f, 1.0f);
        c.titleBgActive = ImVec4(0.22f, 0.28f, 0.16f, 1.0f);
        c.menuBarBg = ImVec4(0.12f, 0.15f, 0.09f, 1.0f);
        c.scrollbarBg = ImVec4(0.10f, 0.12f, 0.08f, 1.0f);
        c.scrollbarGrab = ImVec4(0.35f, 0.42f, 0.25f, 1.0f);
        c.checkMark = ImVec4(0.76f, 0.65f, 0.20f, 1.0f);
        c.sliderGrab = ImVec4(0.60f, 0.50f, 0.15f, 1.0f);
        c.button = ImVec4(0.30f, 0.38f, 0.20f, 1.0f);
        c.buttonHovered = ImVec4(0.40f, 0.50f, 0.28f, 1.0f);
        c.buttonActive = ImVec4(0.50f, 0.60f, 0.35f, 1.0f);
        c.header = ImVec4(0.25f, 0.32f, 0.18f, 1.0f);
        c.headerHovered = ImVec4(0.35f, 0.45f, 0.25f, 1.0f);
        c.headerActive = ImVec4(0.45f, 0.55f, 0.32f, 1.0f);
        c.separator = ImVec4(0.35f, 0.42f, 0.25f, 0.5f);
        c.tab = ImVec4(0.18f, 0.22f, 0.14f, 1.0f);
        c.tabHovered = ImVec4(0.40f, 0.50f, 0.28f, 1.0f);
        c.tabActive = ImVec4(0.35f, 0.42f, 0.25f, 1.0f);
        c.text = ImVec4(0.90f, 0.88f, 0.80f, 1.0f);
        c.textDisabled = ImVec4(0.50f, 0.48f, 0.42f, 1.0f);
        c.plotLines = ImVec4(0.76f, 0.65f, 0.20f, 1.0f);
        c.plotHistogram = ImVec4(0.60f, 0.50f, 0.15f, 1.0f);
        c.tableHeaderBg = ImVec4(0.20f, 0.25f, 0.15f, 1.0f);
        c.tableRowBg = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
        c.tableRowBgAlt = ImVec4(1.0f, 1.0f, 1.0f, 0.03f);
        c.success = ImVec4(0.40f, 0.75f, 0.30f, 1.0f);
        c.warning = ImVec4(0.85f, 0.70f, 0.20f, 1.0f);
        c.error = ImVec4(0.85f, 0.25f, 0.20f, 1.0f);
        c.info = ImVec4(0.40f, 0.65f, 0.80f, 1.0f);
        c.accent = ImVec4(0.76f, 0.65f, 0.20f, 1.0f);

        m_themes.push_back(t);
    }

    // ⚡ Cyberpunk
    {
        ThemeDefinition t;
        t.id = "cyberpunk";
        t.name = "Cyberpunk";
        t.emoji = "⚡";
        t.description = "High contrast neon yellow and pink on pure black";

        auto& c = t.colors;
        c.windowBg = ImVec4(0.02f, 0.02f, 0.04f, 1.0f);
        c.childBg = ImVec4(0.04f, 0.04f, 0.06f, 1.0f);
        c.popupBg = ImVec4(0.05f, 0.05f, 0.08f, 0.98f);
        c.border = ImVec4(1.0f, 0.92f, 0.0f, 0.4f);
        c.frameBg = ImVec4(0.08f, 0.08f, 0.12f, 1.0f);
        c.frameBgHovered = ImVec4(0.15f, 0.10f, 0.20f, 1.0f);
        c.frameBgActive = ImVec4(0.90f, 0.10f, 0.50f, 0.6f);
        c.titleBg = ImVec4(0.02f, 0.02f, 0.04f, 1.0f);
        c.titleBgActive = ImVec4(0.90f, 0.10f, 0.50f, 0.8f);
        c.menuBarBg = ImVec4(0.03f, 0.03f, 0.05f, 1.0f);
        c.scrollbarBg = ImVec4(0.02f, 0.02f, 0.04f, 1.0f);
        c.scrollbarGrab = ImVec4(1.0f, 0.92f, 0.0f, 0.6f);
        c.checkMark = ImVec4(1.0f, 0.92f, 0.0f, 1.0f);
        c.sliderGrab = ImVec4(0.90f, 0.10f, 0.50f, 1.0f);
        c.button = ImVec4(0.90f, 0.10f, 0.50f, 0.6f);
        c.buttonHovered = ImVec4(1.0f, 0.20f, 0.60f, 0.8f);
        c.buttonActive = ImVec4(1.0f, 0.92f, 0.0f, 0.8f);
        c.header = ImVec4(0.90f, 0.10f, 0.50f, 0.3f);
        c.headerHovered = ImVec4(0.90f, 0.10f, 0.50f, 0.6f);
        c.headerActive = ImVec4(0.90f, 0.10f, 0.50f, 0.8f);
        c.separator = ImVec4(1.0f, 0.92f, 0.0f, 0.3f);
        c.tab = ImVec4(0.08f, 0.08f, 0.12f, 1.0f);
        c.tabHovered = ImVec4(0.90f, 0.10f, 0.50f, 0.6f);
        c.tabActive = ImVec4(0.90f, 0.10f, 0.50f, 0.8f);
        c.text = ImVec4(1.0f, 0.95f, 0.85f, 1.0f);
        c.textDisabled = ImVec4(0.40f, 0.40f, 0.45f, 1.0f);
        c.plotLines = ImVec4(1.0f, 0.92f, 0.0f, 1.0f);
        c.plotHistogram = ImVec4(0.90f, 0.10f, 0.50f, 1.0f);
        c.tableHeaderBg = ImVec4(0.08f, 0.08f, 0.12f, 1.0f);
        c.tableRowBg = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
        c.tableRowBgAlt = ImVec4(1.0f, 1.0f, 1.0f, 0.02f);
        c.success = ImVec4(0.0f, 1.0f, 0.50f, 1.0f);
        c.warning = ImVec4(1.0f, 0.92f, 0.0f, 1.0f);
        c.error = ImVec4(1.0f, 0.10f, 0.20f, 1.0f);
        c.info = ImVec4(0.0f, 0.80f, 1.0f, 1.0f);
        c.accent = ImVec4(1.0f, 0.92f, 0.0f, 1.0f);

        m_themes.push_back(t);
    }

    // 🌑 Dark Mode
    {
        ThemeDefinition t;
        t.id = "dark";
        t.name = "Dark Mode";
        t.emoji = "🌑";
        t.description = "Neutral dark grey standard dark theme";

        auto& c = t.colors;
        c.windowBg = ImVec4(0.12f, 0.12f, 0.14f, 1.0f);
        c.childBg = ImVec4(0.14f, 0.14f, 0.16f, 1.0f);
        c.popupBg = ImVec4(0.13f, 0.13f, 0.15f, 0.98f);
        c.border = ImVec4(0.30f, 0.30f, 0.35f, 0.5f);
        c.frameBg = ImVec4(0.18f, 0.18f, 0.21f, 1.0f);
        c.frameBgHovered = ImVec4(0.25f, 0.25f, 0.30f, 1.0f);
        c.frameBgActive = ImVec4(0.30f, 0.30f, 0.38f, 1.0f);
        c.titleBg = ImVec4(0.08f, 0.08f, 0.10f, 1.0f);
        c.titleBgActive = ImVec4(0.16f, 0.16f, 0.20f, 1.0f);
        c.menuBarBg = ImVec4(0.10f, 0.10f, 0.12f, 1.0f);
        c.scrollbarBg = ImVec4(0.10f, 0.10f, 0.12f, 1.0f);
        c.scrollbarGrab = ImVec4(0.35f, 0.35f, 0.40f, 1.0f);
        c.checkMark = ImVec4(0.40f, 0.65f, 0.95f, 1.0f);
        c.sliderGrab = ImVec4(0.40f, 0.65f, 0.95f, 1.0f);
        c.button = ImVec4(0.22f, 0.22f, 0.28f, 1.0f);
        c.buttonHovered = ImVec4(0.30f, 0.30f, 0.40f, 1.0f);
        c.buttonActive = ImVec4(0.40f, 0.40f, 0.55f, 1.0f);
        c.header = ImVec4(0.22f, 0.22f, 0.28f, 1.0f);
        c.headerHovered = ImVec4(0.30f, 0.30f, 0.40f, 1.0f);
        c.headerActive = ImVec4(0.35f, 0.35f, 0.48f, 1.0f);
        c.separator = ImVec4(0.30f, 0.30f, 0.35f, 0.5f);
        c.tab = ImVec4(0.16f, 0.16f, 0.20f, 1.0f);
        c.tabHovered = ImVec4(0.30f, 0.30f, 0.40f, 1.0f);
        c.tabActive = ImVec4(0.25f, 0.25f, 0.35f, 1.0f);
        c.text = ImVec4(0.92f, 0.92f, 0.94f, 1.0f);
        c.textDisabled = ImVec4(0.50f, 0.50f, 0.52f, 1.0f);
        c.plotLines = ImVec4(0.40f, 0.65f, 0.95f, 1.0f);
        c.plotHistogram = ImVec4(0.50f, 0.75f, 0.40f, 1.0f);
        c.tableHeaderBg = ImVec4(0.18f, 0.18f, 0.21f, 1.0f);
        c.tableRowBg = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
        c.tableRowBgAlt = ImVec4(1.0f, 1.0f, 1.0f, 0.03f);
        c.success = ImVec4(0.30f, 0.80f, 0.40f, 1.0f);
        c.warning = ImVec4(0.95f, 0.75f, 0.20f, 1.0f);
        c.error = ImVec4(0.90f, 0.30f, 0.25f, 1.0f);
        c.info = ImVec4(0.40f, 0.65f, 0.95f, 1.0f);
        c.accent = ImVec4(0.40f, 0.65f, 0.95f, 1.0f);

        m_themes.push_back(t);
    }

    // 🤍 White Mode
    {
        ThemeDefinition t;
        t.id = "light";
        t.name = "White Mode";
        t.emoji = "🤍";
        t.description = "Clean minimal light theme with soft shadows";

        auto& c = t.colors;
        c.windowBg = ImVec4(0.96f, 0.96f, 0.97f, 1.0f);
        c.childBg = ImVec4(0.98f, 0.98f, 0.99f, 1.0f);
        c.popupBg = ImVec4(0.97f, 0.97f, 0.98f, 0.98f);
        c.border = ImVec4(0.80f, 0.80f, 0.82f, 1.0f);
        c.frameBg = ImVec4(0.90f, 0.90f, 0.92f, 1.0f);
        c.frameBgHovered = ImVec4(0.85f, 0.85f, 0.88f, 1.0f);
        c.frameBgActive = ImVec4(0.78f, 0.78f, 0.82f, 1.0f);
        c.titleBg = ImVec4(0.92f, 0.92f, 0.94f, 1.0f);
        c.titleBgActive = ImVec4(0.85f, 0.85f, 0.90f, 1.0f);
        c.menuBarBg = ImVec4(0.94f, 0.94f, 0.95f, 1.0f);
        c.scrollbarBg = ImVec4(0.92f, 0.92f, 0.94f, 1.0f);
        c.scrollbarGrab = ImVec4(0.70f, 0.70f, 0.75f, 1.0f);
        c.checkMark = ImVec4(0.20f, 0.50f, 0.90f, 1.0f);
        c.sliderGrab = ImVec4(0.20f, 0.50f, 0.90f, 1.0f);
        c.button = ImVec4(0.85f, 0.85f, 0.88f, 1.0f);
        c.buttonHovered = ImVec4(0.75f, 0.75f, 0.80f, 1.0f);
        c.buttonActive = ImVec4(0.20f, 0.50f, 0.90f, 0.8f);
        c.header = ImVec4(0.88f, 0.88f, 0.90f, 1.0f);
        c.headerHovered = ImVec4(0.80f, 0.82f, 0.88f, 1.0f);
        c.headerActive = ImVec4(0.72f, 0.76f, 0.86f, 1.0f);
        c.separator = ImVec4(0.82f, 0.82f, 0.84f, 1.0f);
        c.tab = ImVec4(0.90f, 0.90f, 0.92f, 1.0f);
        c.tabHovered = ImVec4(0.80f, 0.82f, 0.88f, 1.0f);
        c.tabActive = ImVec4(0.95f, 0.95f, 0.97f, 1.0f);
        c.text = ImVec4(0.10f, 0.10f, 0.12f, 1.0f);
        c.textDisabled = ImVec4(0.55f, 0.55f, 0.58f, 1.0f);
        c.plotLines = ImVec4(0.20f, 0.50f, 0.90f, 1.0f);
        c.plotHistogram = ImVec4(0.30f, 0.70f, 0.40f, 1.0f);
        c.tableHeaderBg = ImVec4(0.88f, 0.88f, 0.90f, 1.0f);
        c.tableRowBg = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
        c.tableRowBgAlt = ImVec4(0.0f, 0.0f, 0.0f, 0.03f);
        c.success = ImVec4(0.20f, 0.70f, 0.30f, 1.0f);
        c.warning = ImVec4(0.85f, 0.65f, 0.10f, 1.0f);
        c.error = ImVec4(0.85f, 0.20f, 0.15f, 1.0f);
        c.info = ImVec4(0.20f, 0.50f, 0.90f, 1.0f);
        c.accent = ImVec4(0.20f, 0.50f, 0.90f, 1.0f);

        m_themes.push_back(t);
    }

    // 💙 Neon Blue
    {
        ThemeDefinition t;
        t.id = "neonblue";
        t.name = "Neon Blue";
        t.emoji = "💙";
        t.description = "Electric blue glows on deep navy";
        auto& c = t.colors;
        c.windowBg = ImVec4(0.05f, 0.06f, 0.14f, 1.0f);
        c.childBg = ImVec4(0.06f, 0.07f, 0.16f, 1.0f);
        c.popupBg = ImVec4(0.06f, 0.07f, 0.15f, 0.98f);
        c.border = ImVec4(0.0f, 0.50f, 1.0f, 0.3f);
        c.frameBg = ImVec4(0.08f, 0.10f, 0.22f, 1.0f);
        c.frameBgHovered = ImVec4(0.12f, 0.18f, 0.35f, 1.0f);
        c.frameBgActive = ImVec4(0.0f, 0.40f, 0.90f, 0.5f);
        c.titleBg = ImVec4(0.04f, 0.05f, 0.10f, 1.0f);
        c.titleBgActive = ImVec4(0.0f, 0.30f, 0.70f, 0.8f);
        c.menuBarBg = ImVec4(0.05f, 0.06f, 0.12f, 1.0f);
        c.scrollbarBg = ImVec4(0.04f, 0.05f, 0.10f, 1.0f);
        c.scrollbarGrab = ImVec4(0.0f, 0.50f, 1.0f, 0.5f);
        c.checkMark = ImVec4(0.0f, 0.70f, 1.0f, 1.0f);
        c.sliderGrab = ImVec4(0.0f, 0.60f, 1.0f, 1.0f);
        c.button = ImVec4(0.0f, 0.35f, 0.75f, 0.6f);
        c.buttonHovered = ImVec4(0.0f, 0.50f, 1.0f, 0.7f);
        c.buttonActive = ImVec4(0.0f, 0.60f, 1.0f, 0.9f);
        c.header = ImVec4(0.0f, 0.30f, 0.65f, 0.4f);
        c.headerHovered = ImVec4(0.0f, 0.40f, 0.80f, 0.6f);
        c.headerActive = ImVec4(0.0f, 0.50f, 1.0f, 0.7f);
        c.separator = ImVec4(0.0f, 0.50f, 1.0f, 0.2f);
        c.tab = ImVec4(0.06f, 0.08f, 0.18f, 1.0f);
        c.tabHovered = ImVec4(0.0f, 0.40f, 0.85f, 0.6f);
        c.tabActive = ImVec4(0.0f, 0.35f, 0.75f, 0.8f);
        c.text = ImVec4(0.85f, 0.90f, 1.0f, 1.0f);
        c.textDisabled = ImVec4(0.35f, 0.40f, 0.55f, 1.0f);
        c.plotLines = ImVec4(0.0f, 0.70f, 1.0f, 1.0f);
        c.plotHistogram = ImVec4(0.0f, 0.50f, 1.0f, 1.0f);
        c.tableHeaderBg = ImVec4(0.08f, 0.10f, 0.22f, 1.0f);
        c.tableRowBg = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
        c.tableRowBgAlt = ImVec4(0.0f, 0.0f, 0.20f, 0.08f);
        c.success = ImVec4(0.0f, 0.85f, 0.50f, 1.0f);
        c.warning = ImVec4(1.0f, 0.80f, 0.0f, 1.0f);
        c.error = ImVec4(1.0f, 0.20f, 0.25f, 1.0f);
        c.info = ImVec4(0.0f, 0.70f, 1.0f, 1.0f);
        c.accent = ImVec4(0.0f, 0.70f, 1.0f, 1.0f);

        m_themes.push_back(t);
    }

    // Additional themes registered with same pattern...
    // 🌋 Lava, 🌸 Sakura, 🌊 Deep Ocean, ☢️ Toxic, 👾 Retro Terminal, etc.
    // Adding a few more key ones:

    // 🌋 Lava
    {
        ThemeDefinition t;
        t.id = "lava";
        t.name = "Lava";
        t.emoji = "🌋";
        t.description = "Deep black with molten orange and red glowing accents";
        auto& c = t.colors;
        c.windowBg = ImVec4(0.06f, 0.04f, 0.04f, 1.0f);
        c.childBg = ImVec4(0.08f, 0.05f, 0.05f, 1.0f);
        c.popupBg = ImVec4(0.07f, 0.05f, 0.05f, 0.98f);
        c.border = ImVec4(0.90f, 0.35f, 0.05f, 0.3f);
        c.frameBg = ImVec4(0.15f, 0.08f, 0.06f, 1.0f);
        c.frameBgHovered = ImVec4(0.25f, 0.12f, 0.08f, 1.0f);
        c.frameBgActive = ImVec4(0.80f, 0.30f, 0.05f, 0.5f);
        c.titleBg = ImVec4(0.04f, 0.02f, 0.02f, 1.0f);
        c.titleBgActive = ImVec4(0.70f, 0.20f, 0.05f, 0.8f);
        c.menuBarBg = ImVec4(0.05f, 0.03f, 0.03f, 1.0f);
        c.scrollbarBg = ImVec4(0.04f, 0.02f, 0.02f, 1.0f);
        c.scrollbarGrab = ImVec4(0.80f, 0.30f, 0.05f, 0.5f);
        c.checkMark = ImVec4(1.0f, 0.55f, 0.0f, 1.0f);
        c.sliderGrab = ImVec4(0.90f, 0.40f, 0.05f, 1.0f);
        c.button = ImVec4(0.60f, 0.15f, 0.05f, 0.7f);
        c.buttonHovered = ImVec4(0.80f, 0.25f, 0.05f, 0.8f);
        c.buttonActive = ImVec4(1.0f, 0.45f, 0.05f, 0.9f);
        c.header = ImVec4(0.50f, 0.12f, 0.05f, 0.5f);
        c.headerHovered = ImVec4(0.70f, 0.20f, 0.05f, 0.6f);
        c.headerActive = ImVec4(0.85f, 0.30f, 0.05f, 0.7f);
        c.separator = ImVec4(0.90f, 0.35f, 0.05f, 0.2f);
        c.tab = ImVec4(0.12f, 0.06f, 0.04f, 1.0f);
        c.tabHovered = ImVec4(0.70f, 0.20f, 0.05f, 0.6f);
        c.tabActive = ImVec4(0.60f, 0.15f, 0.05f, 0.8f);
        c.text = ImVec4(1.0f, 0.90f, 0.80f, 1.0f);
        c.textDisabled = ImVec4(0.50f, 0.35f, 0.30f, 1.0f);
        c.plotLines = ImVec4(1.0f, 0.55f, 0.0f, 1.0f);
        c.plotHistogram = ImVec4(0.90f, 0.30f, 0.05f, 1.0f);
        c.tableHeaderBg = ImVec4(0.15f, 0.08f, 0.06f, 1.0f);
        c.tableRowBg = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
        c.tableRowBgAlt = ImVec4(0.20f, 0.05f, 0.02f, 0.08f);
        c.success = ImVec4(0.20f, 0.80f, 0.30f, 1.0f);
        c.warning = ImVec4(1.0f, 0.70f, 0.0f, 1.0f);
        c.error = ImVec4(1.0f, 0.15f, 0.10f, 1.0f);
        c.info = ImVec4(1.0f, 0.55f, 0.0f, 1.0f);
        c.accent = ImVec4(1.0f, 0.45f, 0.0f, 1.0f);

        m_themes.push_back(t);
    }

    // 👾 Retro Terminal
    {
        ThemeDefinition t;
        t.id = "retro_terminal";
        t.name = "Retro Terminal";
        t.emoji = "👾";
        t.description = "Amber or green phosphor on black CRT";
        t.windowRounding = 0.0f;
        t.frameRounding = 0.0f;
        t.grabRounding = 0.0f;
        t.tabRounding = 0.0f;

        auto& c = t.colors;
        ImVec4 phosphor(0.0f, 0.90f, 0.20f, 1.0f); // green
        ImVec4 dim(0.0f, 0.40f, 0.10f, 1.0f);
        c.windowBg = ImVec4(0.0f, 0.02f, 0.0f, 1.0f);
        c.childBg = ImVec4(0.0f, 0.03f, 0.01f, 1.0f);
        c.popupBg = ImVec4(0.0f, 0.04f, 0.01f, 0.98f);
        c.border = ImVec4(0.0f, 0.50f, 0.12f, 0.4f);
        c.frameBg = ImVec4(0.0f, 0.06f, 0.02f, 1.0f);
        c.frameBgHovered = ImVec4(0.0f, 0.12f, 0.04f, 1.0f);
        c.frameBgActive = ImVec4(0.0f, 0.20f, 0.06f, 1.0f);
        c.titleBg = ImVec4(0.0f, 0.01f, 0.0f, 1.0f);
        c.titleBgActive = ImVec4(0.0f, 0.15f, 0.04f, 1.0f);
        c.menuBarBg = ImVec4(0.0f, 0.02f, 0.0f, 1.0f);
        c.scrollbarBg = ImVec4(0.0f, 0.01f, 0.0f, 1.0f);
        c.scrollbarGrab = phosphor;
        c.checkMark = phosphor;
        c.sliderGrab = phosphor;
        c.button = ImVec4(0.0f, 0.15f, 0.04f, 1.0f);
        c.buttonHovered = ImVec4(0.0f, 0.25f, 0.08f, 1.0f);
        c.buttonActive = ImVec4(0.0f, 0.40f, 0.12f, 1.0f);
        c.header = ImVec4(0.0f, 0.12f, 0.04f, 1.0f);
        c.headerHovered = ImVec4(0.0f, 0.20f, 0.06f, 1.0f);
        c.headerActive = ImVec4(0.0f, 0.30f, 0.10f, 1.0f);
        c.separator = ImVec4(0.0f, 0.40f, 0.10f, 0.3f);
        c.tab = ImVec4(0.0f, 0.06f, 0.02f, 1.0f);
        c.tabHovered = ImVec4(0.0f, 0.20f, 0.06f, 1.0f);
        c.tabActive = ImVec4(0.0f, 0.15f, 0.04f, 1.0f);
        c.text = phosphor;
        c.textDisabled = dim;
        c.plotLines = phosphor;
        c.plotHistogram = ImVec4(0.0f, 0.70f, 0.18f, 1.0f);
        c.tableHeaderBg = ImVec4(0.0f, 0.06f, 0.02f, 1.0f);
        c.tableRowBg = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
        c.tableRowBgAlt = ImVec4(0.0f, 0.06f, 0.02f, 0.05f);
        c.success = phosphor;
        c.warning = ImVec4(0.80f, 0.80f, 0.0f, 1.0f);
        c.error = ImVec4(0.90f, 0.20f, 0.10f, 1.0f);
        c.info = phosphor;
        c.accent = phosphor;

        m_themes.push_back(t);
    }

    // 🌸 Sakura
    {
        ThemeDefinition t;
        t.id = "sakura";
        t.name = "Sakura";
        t.emoji = "🌸";
        t.description = "Soft pink and white Japanese blossom light theme";
        t.windowRounding = 10.0f;
        t.frameRounding = 6.0f;

        auto& c = t.colors;
        c.windowBg = ImVec4(0.99f, 0.96f, 0.97f, 1.0f);
        c.childBg = ImVec4(1.0f, 0.97f, 0.98f, 1.0f);
        c.popupBg = ImVec4(1.0f, 0.97f, 0.98f, 0.98f);
        c.border = ImVec4(0.90f, 0.72f, 0.78f, 0.5f);
        c.frameBg = ImVec4(0.96f, 0.90f, 0.92f, 1.0f);
        c.frameBgHovered = ImVec4(0.94f, 0.84f, 0.88f, 1.0f);
        c.frameBgActive = ImVec4(0.90f, 0.70f, 0.78f, 1.0f);
        c.titleBg = ImVec4(0.95f, 0.88f, 0.90f, 1.0f);
        c.titleBgActive = ImVec4(0.90f, 0.72f, 0.78f, 1.0f);
        c.menuBarBg = ImVec4(0.97f, 0.92f, 0.94f, 1.0f);
        c.scrollbarBg = ImVec4(0.96f, 0.90f, 0.92f, 1.0f);
        c.scrollbarGrab = ImVec4(0.85f, 0.60f, 0.70f, 1.0f);
        c.checkMark = ImVec4(0.85f, 0.40f, 0.55f, 1.0f);
        c.sliderGrab = ImVec4(0.85f, 0.45f, 0.58f, 1.0f);
        c.button = ImVec4(0.92f, 0.75f, 0.82f, 1.0f);
        c.buttonHovered = ImVec4(0.88f, 0.62f, 0.72f, 1.0f);
        c.buttonActive = ImVec4(0.85f, 0.45f, 0.58f, 1.0f);
        c.header = ImVec4(0.94f, 0.82f, 0.87f, 1.0f);
        c.headerHovered = ImVec4(0.90f, 0.72f, 0.78f, 1.0f);
        c.headerActive = ImVec4(0.85f, 0.60f, 0.70f, 1.0f);
        c.separator = ImVec4(0.90f, 0.72f, 0.78f, 0.5f);
        c.tab = ImVec4(0.96f, 0.88f, 0.91f, 1.0f);
        c.tabHovered = ImVec4(0.90f, 0.72f, 0.78f, 1.0f);
        c.tabActive = ImVec4(0.99f, 0.96f, 0.97f, 1.0f);
        c.text = ImVec4(0.30f, 0.18f, 0.22f, 1.0f);
        c.textDisabled = ImVec4(0.65f, 0.55f, 0.58f, 1.0f);
        c.plotLines = ImVec4(0.85f, 0.40f, 0.55f, 1.0f);
        c.plotHistogram = ImVec4(0.85f, 0.55f, 0.65f, 1.0f);
        c.tableHeaderBg = ImVec4(0.94f, 0.85f, 0.88f, 1.0f);
        c.tableRowBg = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
        c.tableRowBgAlt = ImVec4(0.85f, 0.60f, 0.70f, 0.04f);
        c.success = ImVec4(0.40f, 0.75f, 0.50f, 1.0f);
        c.warning = ImVec4(0.90f, 0.70f, 0.20f, 1.0f);
        c.error = ImVec4(0.85f, 0.25f, 0.30f, 1.0f);
        c.info = ImVec4(0.45f, 0.60f, 0.85f, 1.0f);
        c.accent = ImVec4(0.85f, 0.40f, 0.55f, 1.0f);

        m_themes.push_back(t);
    }

    // 🌊 Deep Ocean
    {
        ThemeDefinition t;
        t.id = "deep_ocean";
        t.name = "Deep Ocean";
        t.emoji = "🌊";
        t.description = "Dark teal and bioluminescent green underwater theme";

        auto& c = t.colors;
        c.windowBg = ImVec4(0.03f, 0.08f, 0.12f, 1.0f);
        c.childBg = ImVec4(0.04f, 0.10f, 0.14f, 1.0f);
        c.popupBg = ImVec4(0.04f, 0.09f, 0.13f, 0.98f);
        c.border = ImVec4(0.0f, 0.65f, 0.60f, 0.3f);
        c.frameBg = ImVec4(0.05f, 0.14f, 0.20f, 1.0f);
        c.frameBgHovered = ImVec4(0.08f, 0.22f, 0.28f, 1.0f);
        c.frameBgActive = ImVec4(0.0f, 0.55f, 0.50f, 0.5f);
        c.titleBg = ImVec4(0.02f, 0.06f, 0.09f, 1.0f);
        c.titleBgActive = ImVec4(0.0f, 0.40f, 0.38f, 0.7f);
        c.menuBarBg = ImVec4(0.03f, 0.07f, 0.10f, 1.0f);
        c.scrollbarBg = ImVec4(0.02f, 0.06f, 0.09f, 1.0f);
        c.scrollbarGrab = ImVec4(0.0f, 0.55f, 0.50f, 0.5f);
        c.checkMark = ImVec4(0.0f, 0.90f, 0.70f, 1.0f);
        c.sliderGrab = ImVec4(0.0f, 0.75f, 0.60f, 1.0f);
        c.button = ImVec4(0.0f, 0.35f, 0.35f, 0.6f);
        c.buttonHovered = ImVec4(0.0f, 0.50f, 0.48f, 0.7f);
        c.buttonActive = ImVec4(0.0f, 0.65f, 0.58f, 0.9f);
        c.header = ImVec4(0.0f, 0.28f, 0.30f, 0.5f);
        c.headerHovered = ImVec4(0.0f, 0.40f, 0.38f, 0.6f);
        c.headerActive = ImVec4(0.0f, 0.55f, 0.50f, 0.7f);
        c.separator = ImVec4(0.0f, 0.55f, 0.50f, 0.2f);
        c.tab = ImVec4(0.04f, 0.10f, 0.14f, 1.0f);
        c.tabHovered = ImVec4(0.0f, 0.40f, 0.38f, 0.6f);
        c.tabActive = ImVec4(0.0f, 0.30f, 0.30f, 0.8f);
        c.text = ImVec4(0.78f, 0.95f, 0.92f, 1.0f);
        c.textDisabled = ImVec4(0.30f, 0.45f, 0.42f, 1.0f);
        c.plotLines = ImVec4(0.0f, 0.90f, 0.70f, 1.0f);
        c.plotHistogram = ImVec4(0.0f, 0.65f, 0.55f, 1.0f);
        c.tableHeaderBg = ImVec4(0.05f, 0.14f, 0.20f, 1.0f);
        c.tableRowBg = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
        c.tableRowBgAlt = ImVec4(0.0f, 0.20f, 0.18f, 0.06f);
        c.success = ImVec4(0.0f, 0.90f, 0.50f, 1.0f);
        c.warning = ImVec4(0.95f, 0.80f, 0.15f, 1.0f);
        c.error = ImVec4(0.90f, 0.25f, 0.20f, 1.0f);
        c.info = ImVec4(0.0f, 0.80f, 0.70f, 1.0f);
        c.accent = ImVec4(0.0f, 0.90f, 0.70f, 1.0f);

        m_themes.push_back(t);
    }

    // ☢️ Toxic
    {
        ThemeDefinition t;
        t.id = "toxic";
        t.name = "Toxic";
        t.emoji = "☢️";
        t.description = "Acid green on black hazmat warning theme";

        auto& c = t.colors;
        c.windowBg = ImVec4(0.03f, 0.03f, 0.02f, 1.0f);
        c.childBg = ImVec4(0.04f, 0.04f, 0.03f, 1.0f);
        c.popupBg = ImVec4(0.04f, 0.04f, 0.03f, 0.98f);
        c.border = ImVec4(0.50f, 0.90f, 0.0f, 0.3f);
        c.frameBg = ImVec4(0.08f, 0.10f, 0.04f, 1.0f);
        c.frameBgHovered = ImVec4(0.15f, 0.20f, 0.06f, 1.0f);
        c.frameBgActive = ImVec4(0.45f, 0.80f, 0.0f, 0.4f);
        c.titleBg = ImVec4(0.02f, 0.02f, 0.01f, 1.0f);
        c.titleBgActive = ImVec4(0.30f, 0.55f, 0.0f, 0.7f);
        c.menuBarBg = ImVec4(0.03f, 0.03f, 0.02f, 1.0f);
        c.scrollbarBg = ImVec4(0.02f, 0.02f, 0.01f, 1.0f);
        c.scrollbarGrab = ImVec4(0.50f, 0.90f, 0.0f, 0.5f);
        c.checkMark = ImVec4(0.55f, 1.0f, 0.0f, 1.0f);
        c.sliderGrab = ImVec4(0.50f, 0.90f, 0.0f, 1.0f);
        c.button = ImVec4(0.25f, 0.45f, 0.0f, 0.6f);
        c.buttonHovered = ImVec4(0.35f, 0.65f, 0.0f, 0.7f);
        c.buttonActive = ImVec4(0.50f, 0.90f, 0.0f, 0.9f);
        c.header = ImVec4(0.20f, 0.35f, 0.0f, 0.4f);
        c.headerHovered = ImVec4(0.30f, 0.55f, 0.0f, 0.6f);
        c.headerActive = ImVec4(0.40f, 0.75f, 0.0f, 0.7f);
        c.separator = ImVec4(0.50f, 0.90f, 0.0f, 0.2f);
        c.tab = ImVec4(0.06f, 0.08f, 0.03f, 1.0f);
        c.tabHovered = ImVec4(0.30f, 0.55f, 0.0f, 0.6f);
        c.tabActive = ImVec4(0.25f, 0.45f, 0.0f, 0.8f);
        c.text = ImVec4(0.55f, 1.0f, 0.0f, 1.0f);
        c.textDisabled = ImVec4(0.25f, 0.45f, 0.0f, 1.0f);
        c.plotLines = ImVec4(0.55f, 1.0f, 0.0f, 1.0f);
        c.plotHistogram = ImVec4(0.45f, 0.80f, 0.0f, 1.0f);
        c.tableHeaderBg = ImVec4(0.08f, 0.10f, 0.04f, 1.0f);
        c.tableRowBg = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
        c.tableRowBgAlt = ImVec4(0.20f, 0.30f, 0.0f, 0.05f);
        c.success = ImVec4(0.55f, 1.0f, 0.0f, 1.0f);
        c.warning = ImVec4(1.0f, 0.80f, 0.0f, 1.0f);
        c.error = ImVec4(1.0f, 0.15f, 0.10f, 1.0f);
        c.info = ImVec4(0.55f, 1.0f, 0.0f, 1.0f);
        c.accent = ImVec4(0.55f, 1.0f, 0.0f, 1.0f);

        m_themes.push_back(t);
    }

    // 🌆 Synthwave
    {
        ThemeDefinition t;
        t.id = "synthwave";
        t.name = "Synthwave";
        t.emoji = "🌆";
        t.description = "Purple and pink retro 80s sunset with grid lines";

        auto& c = t.colors;
        c.windowBg = ImVec4(0.08f, 0.02f, 0.15f, 1.0f);
        c.childBg = ImVec4(0.10f, 0.03f, 0.18f, 1.0f);
        c.popupBg = ImVec4(0.09f, 0.03f, 0.16f, 0.98f);
        c.border = ImVec4(0.90f, 0.20f, 0.80f, 0.3f);
        c.frameBg = ImVec4(0.15f, 0.05f, 0.25f, 1.0f);
        c.frameBgHovered = ImVec4(0.22f, 0.08f, 0.35f, 1.0f);
        c.frameBgActive = ImVec4(0.80f, 0.15f, 0.70f, 0.5f);
        c.titleBg = ImVec4(0.06f, 0.01f, 0.10f, 1.0f);
        c.titleBgActive = ImVec4(0.60f, 0.10f, 0.55f, 0.8f);
        c.menuBarBg = ImVec4(0.07f, 0.02f, 0.12f, 1.0f);
        c.scrollbarBg = ImVec4(0.06f, 0.01f, 0.10f, 1.0f);
        c.scrollbarGrab = ImVec4(0.80f, 0.15f, 0.70f, 0.5f);
        c.checkMark = ImVec4(1.0f, 0.40f, 0.80f, 1.0f);
        c.sliderGrab = ImVec4(0.90f, 0.25f, 0.75f, 1.0f);
        c.button = ImVec4(0.55f, 0.10f, 0.50f, 0.6f);
        c.buttonHovered = ImVec4(0.75f, 0.15f, 0.65f, 0.7f);
        c.buttonActive = ImVec4(1.0f, 0.30f, 0.85f, 0.9f);
        c.header = ImVec4(0.45f, 0.08f, 0.40f, 0.4f);
        c.headerHovered = ImVec4(0.60f, 0.12f, 0.55f, 0.6f);
        c.headerActive = ImVec4(0.75f, 0.15f, 0.65f, 0.7f);
        c.separator = ImVec4(0.80f, 0.15f, 0.70f, 0.2f);
        c.tab = ImVec4(0.12f, 0.04f, 0.20f, 1.0f);
        c.tabHovered = ImVec4(0.60f, 0.10f, 0.55f, 0.6f);
        c.tabActive = ImVec4(0.50f, 0.08f, 0.45f, 0.8f);
        c.text = ImVec4(0.95f, 0.85f, 1.0f, 1.0f);
        c.textDisabled = ImVec4(0.45f, 0.30f, 0.50f, 1.0f);
        c.plotLines = ImVec4(1.0f, 0.40f, 0.80f, 1.0f);
        c.plotHistogram = ImVec4(0.80f, 0.20f, 0.70f, 1.0f);
        c.tableHeaderBg = ImVec4(0.15f, 0.05f, 0.25f, 1.0f);
        c.tableRowBg = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
        c.tableRowBgAlt = ImVec4(0.30f, 0.05f, 0.25f, 0.06f);
        c.success = ImVec4(0.20f, 0.90f, 0.60f, 1.0f);
        c.warning = ImVec4(1.0f, 0.80f, 0.15f, 1.0f);
        c.error = ImVec4(1.0f, 0.20f, 0.30f, 1.0f);
        c.info = ImVec4(0.50f, 0.70f, 1.0f, 1.0f);
        c.accent = ImVec4(1.0f, 0.40f, 0.80f, 1.0f);

        m_themes.push_back(t);
    }

    // 🚀 Sci-Fi
    {
        ThemeDefinition t;
        t.id = "scifi";
        t.name = "Sci-Fi";
        t.emoji = "🚀";
        t.description = "Cool steel blues and holographic cyan on near-black";

        auto& c = t.colors;
        c.windowBg = ImVec4(0.05f, 0.06f, 0.08f, 1.0f);
        c.childBg = ImVec4(0.06f, 0.07f, 0.10f, 1.0f);
        c.popupBg = ImVec4(0.06f, 0.07f, 0.09f, 0.98f);
        c.border = ImVec4(0.30f, 0.60f, 0.80f, 0.3f);
        c.frameBg = ImVec4(0.08f, 0.10f, 0.14f, 1.0f);
        c.frameBgHovered = ImVec4(0.12f, 0.16f, 0.22f, 1.0f);
        c.frameBgActive = ImVec4(0.20f, 0.50f, 0.70f, 0.5f);
        c.titleBg = ImVec4(0.04f, 0.04f, 0.06f, 1.0f);
        c.titleBgActive = ImVec4(0.15f, 0.35f, 0.50f, 0.8f);
        c.menuBarBg = ImVec4(0.04f, 0.05f, 0.07f, 1.0f);
        c.scrollbarBg = ImVec4(0.04f, 0.04f, 0.06f, 1.0f);
        c.scrollbarGrab = ImVec4(0.30f, 0.55f, 0.75f, 0.5f);
        c.checkMark = ImVec4(0.0f, 0.85f, 0.95f, 1.0f);
        c.sliderGrab = ImVec4(0.0f, 0.75f, 0.90f, 1.0f);
        c.button = ImVec4(0.12f, 0.28f, 0.40f, 0.6f);
        c.buttonHovered = ImVec4(0.18f, 0.42f, 0.58f, 0.7f);
        c.buttonActive = ImVec4(0.0f, 0.65f, 0.85f, 0.9f);
        c.header = ImVec4(0.10f, 0.22f, 0.32f, 0.5f);
        c.headerHovered = ImVec4(0.15f, 0.35f, 0.48f, 0.6f);
        c.headerActive = ImVec4(0.20f, 0.48f, 0.65f, 0.7f);
        c.separator = ImVec4(0.25f, 0.55f, 0.75f, 0.2f);
        c.tab = ImVec4(0.06f, 0.08f, 0.12f, 1.0f);
        c.tabHovered = ImVec4(0.15f, 0.35f, 0.50f, 0.6f);
        c.tabActive = ImVec4(0.12f, 0.28f, 0.42f, 0.8f);
        c.text = ImVec4(0.82f, 0.90f, 0.95f, 1.0f);
        c.textDisabled = ImVec4(0.35f, 0.42f, 0.48f, 1.0f);
        c.plotLines = ImVec4(0.0f, 0.85f, 0.95f, 1.0f);
        c.plotHistogram = ImVec4(0.20f, 0.60f, 0.80f, 1.0f);
        c.tableHeaderBg = ImVec4(0.08f, 0.10f, 0.14f, 1.0f);
        c.tableRowBg = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
        c.tableRowBgAlt = ImVec4(0.10f, 0.20f, 0.30f, 0.05f);
        c.success = ImVec4(0.0f, 0.85f, 0.50f, 1.0f);
        c.warning = ImVec4(1.0f, 0.80f, 0.15f, 1.0f);
        c.error = ImVec4(0.95f, 0.25f, 0.20f, 1.0f);
        c.info = ImVec4(0.0f, 0.85f, 0.95f, 1.0f);
        c.accent = ImVec4(0.0f, 0.85f, 0.95f, 1.0f);

        m_themes.push_back(t);
    }

    // 📟 Classic
    {
        ThemeDefinition t;
        t.id = "classic";
        t.name = "Classic";
        t.emoji = "📟";
        t.description = "Windows 95 / early 2000s inspired grey and blue";
        t.windowRounding = 0.0f;
        t.frameRounding = 0.0f;
        t.grabRounding = 0.0f;
        t.tabRounding = 0.0f;

        auto& c = t.colors;
        c.windowBg = ImVec4(0.75f, 0.75f, 0.75f, 1.0f);
        c.childBg = ImVec4(0.78f, 0.78f, 0.78f, 1.0f);
        c.popupBg = ImVec4(0.75f, 0.75f, 0.75f, 0.98f);
        c.border = ImVec4(0.50f, 0.50f, 0.50f, 1.0f);
        c.frameBg = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
        c.frameBgHovered = ImVec4(0.90f, 0.90f, 0.95f, 1.0f);
        c.frameBgActive = ImVec4(0.80f, 0.80f, 0.90f, 1.0f);
        c.titleBg = ImVec4(0.0f, 0.0f, 0.50f, 1.0f);
        c.titleBgActive = ImVec4(0.0f, 0.0f, 0.65f, 1.0f);
        c.menuBarBg = ImVec4(0.75f, 0.75f, 0.75f, 1.0f);
        c.scrollbarBg = ImVec4(0.70f, 0.70f, 0.70f, 1.0f);
        c.scrollbarGrab = ImVec4(0.60f, 0.60f, 0.60f, 1.0f);
        c.checkMark = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
        c.sliderGrab = ImVec4(0.0f, 0.0f, 0.50f, 1.0f);
        c.button = ImVec4(0.75f, 0.75f, 0.75f, 1.0f);
        c.buttonHovered = ImVec4(0.82f, 0.82f, 0.82f, 1.0f);
        c.buttonActive = ImVec4(0.65f, 0.65f, 0.65f, 1.0f);
        c.header = ImVec4(0.0f, 0.0f, 0.50f, 0.3f);
        c.headerHovered = ImVec4(0.0f, 0.0f, 0.50f, 0.5f);
        c.headerActive = ImVec4(0.0f, 0.0f, 0.50f, 0.7f);
        c.separator = ImVec4(0.50f, 0.50f, 0.50f, 1.0f);
        c.tab = ImVec4(0.72f, 0.72f, 0.72f, 1.0f);
        c.tabHovered = ImVec4(0.0f, 0.0f, 0.50f, 0.5f);
        c.tabActive = ImVec4(0.78f, 0.78f, 0.78f, 1.0f);
        c.text = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
        c.textDisabled = ImVec4(0.45f, 0.45f, 0.45f, 1.0f);
        c.plotLines = ImVec4(0.0f, 0.0f, 0.50f, 1.0f);
        c.plotHistogram = ImVec4(0.0f, 0.35f, 0.0f, 1.0f);
        c.tableHeaderBg = ImVec4(0.70f, 0.70f, 0.72f, 1.0f);
        c.tableRowBg = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
        c.tableRowBgAlt = ImVec4(0.0f, 0.0f, 0.0f, 0.04f);
        c.success = ImVec4(0.0f, 0.55f, 0.0f, 1.0f);
        c.warning = ImVec4(0.85f, 0.65f, 0.0f, 1.0f);
        c.error = ImVec4(0.80f, 0.0f, 0.0f, 1.0f);
        c.info = ImVec4(0.0f, 0.0f, 0.65f, 1.0f);
        c.accent = ImVec4(0.0f, 0.0f, 0.65f, 1.0f);

        m_themes.push_back(t);
    }

    // 🪖 Military
    {
        ThemeDefinition t;
        t.id = "military";
        t.name = "Military";
        t.emoji = "🪖";
        t.description = "Olive drab green and tan tactical theme";

        auto& c = t.colors;
        c.windowBg = ImVec4(0.15f, 0.16f, 0.12f, 1.0f);
        c.childBg = ImVec4(0.17f, 0.18f, 0.14f, 1.0f);
        c.popupBg = ImVec4(0.16f, 0.17f, 0.13f, 0.98f);
        c.border = ImVec4(0.40f, 0.42f, 0.30f, 0.5f);
        c.frameBg = ImVec4(0.22f, 0.24f, 0.18f, 1.0f);
        c.frameBgHovered = ImVec4(0.28f, 0.30f, 0.22f, 1.0f);
        c.frameBgActive = ImVec4(0.35f, 0.38f, 0.28f, 1.0f);
        c.titleBg = ImVec4(0.12f, 0.13f, 0.10f, 1.0f);
        c.titleBgActive = ImVec4(0.28f, 0.30f, 0.20f, 1.0f);
        c.menuBarBg = ImVec4(0.14f, 0.15f, 0.11f, 1.0f);
        c.scrollbarBg = ImVec4(0.12f, 0.13f, 0.10f, 1.0f);
        c.scrollbarGrab = ImVec4(0.40f, 0.42f, 0.30f, 1.0f);
        c.checkMark = ImVec4(0.75f, 0.68f, 0.45f, 1.0f);
        c.sliderGrab = ImVec4(0.60f, 0.55f, 0.35f, 1.0f);
        c.button = ImVec4(0.30f, 0.32f, 0.22f, 1.0f);
        c.buttonHovered = ImVec4(0.38f, 0.40f, 0.28f, 1.0f);
        c.buttonActive = ImVec4(0.48f, 0.50f, 0.35f, 1.0f);
        c.header = ImVec4(0.28f, 0.30f, 0.20f, 1.0f);
        c.headerHovered = ImVec4(0.35f, 0.38f, 0.26f, 1.0f);
        c.headerActive = ImVec4(0.42f, 0.45f, 0.32f, 1.0f);
        c.separator = ImVec4(0.40f, 0.42f, 0.30f, 0.4f);
        c.tab = ImVec4(0.18f, 0.20f, 0.15f, 1.0f);
        c.tabHovered = ImVec4(0.35f, 0.38f, 0.26f, 1.0f);
        c.tabActive = ImVec4(0.28f, 0.30f, 0.22f, 1.0f);
        c.text = ImVec4(0.88f, 0.85f, 0.75f, 1.0f);
        c.textDisabled = ImVec4(0.50f, 0.48f, 0.40f, 1.0f);
        c.plotLines = ImVec4(0.75f, 0.68f, 0.45f, 1.0f);
        c.plotHistogram = ImVec4(0.55f, 0.50f, 0.30f, 1.0f);
        c.tableHeaderBg = ImVec4(0.22f, 0.24f, 0.18f, 1.0f);
        c.tableRowBg = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
        c.tableRowBgAlt = ImVec4(0.30f, 0.32f, 0.20f, 0.04f);
        c.success = ImVec4(0.40f, 0.70f, 0.30f, 1.0f);
        c.warning = ImVec4(0.85f, 0.70f, 0.20f, 1.0f);
        c.error = ImVec4(0.80f, 0.25f, 0.20f, 1.0f);
        c.info = ImVec4(0.50f, 0.65f, 0.75f, 1.0f);
        c.accent = ImVec4(0.75f, 0.68f, 0.45f, 1.0f);

        m_themes.push_back(t);
    }

    // 🧊 Arctic
    {
        ThemeDefinition t;
        t.id = "arctic";
        t.name = "Arctic";
        t.emoji = "🧊";
        t.description = "Ice white and pale blue ultra clean cold theme";

        auto& c = t.colors;
        c.windowBg = ImVec4(0.94f, 0.96f, 0.99f, 1.0f);
        c.childBg = ImVec4(0.96f, 0.97f, 1.0f, 1.0f);
        c.popupBg = ImVec4(0.95f, 0.97f, 1.0f, 0.98f);
        c.border = ImVec4(0.72f, 0.82f, 0.92f, 0.6f);
        c.frameBg = ImVec4(0.88f, 0.92f, 0.97f, 1.0f);
        c.frameBgHovered = ImVec4(0.82f, 0.88f, 0.95f, 1.0f);
        c.frameBgActive = ImVec4(0.70f, 0.82f, 0.94f, 1.0f);
        c.titleBg = ImVec4(0.86f, 0.90f, 0.96f, 1.0f);
        c.titleBgActive = ImVec4(0.72f, 0.82f, 0.94f, 1.0f);
        c.menuBarBg = ImVec4(0.92f, 0.94f, 0.98f, 1.0f);
        c.scrollbarBg = ImVec4(0.90f, 0.93f, 0.97f, 1.0f);
        c.scrollbarGrab = ImVec4(0.65f, 0.78f, 0.90f, 1.0f);
        c.checkMark = ImVec4(0.20f, 0.55f, 0.85f, 1.0f);
        c.sliderGrab = ImVec4(0.25f, 0.60f, 0.88f, 1.0f);
        c.button = ImVec4(0.80f, 0.88f, 0.96f, 1.0f);
        c.buttonHovered = ImVec4(0.70f, 0.80f, 0.92f, 1.0f);
        c.buttonActive = ImVec4(0.55f, 0.72f, 0.90f, 1.0f);
        c.header = ImVec4(0.82f, 0.88f, 0.95f, 1.0f);
        c.headerHovered = ImVec4(0.72f, 0.82f, 0.92f, 1.0f);
        c.headerActive = ImVec4(0.62f, 0.75f, 0.90f, 1.0f);
        c.separator = ImVec4(0.72f, 0.82f, 0.92f, 0.5f);
        c.tab = ImVec4(0.88f, 0.92f, 0.97f, 1.0f);
        c.tabHovered = ImVec4(0.72f, 0.82f, 0.92f, 1.0f);
        c.tabActive = ImVec4(0.96f, 0.97f, 1.0f, 1.0f);
        c.text = ImVec4(0.12f, 0.18f, 0.28f, 1.0f);
        c.textDisabled = ImVec4(0.50f, 0.58f, 0.68f, 1.0f);
        c.plotLines = ImVec4(0.20f, 0.55f, 0.85f, 1.0f);
        c.plotHistogram = ImVec4(0.35f, 0.65f, 0.90f, 1.0f);
        c.tableHeaderBg = ImVec4(0.85f, 0.90f, 0.96f, 1.0f);
        c.tableRowBg = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
        c.tableRowBgAlt = ImVec4(0.20f, 0.40f, 0.60f, 0.03f);
        c.success = ImVec4(0.20f, 0.70f, 0.45f, 1.0f);
        c.warning = ImVec4(0.85f, 0.65f, 0.15f, 1.0f);
        c.error = ImVec4(0.80f, 0.22f, 0.25f, 1.0f);
        c.info = ImVec4(0.20f, 0.55f, 0.85f, 1.0f);
        c.accent = ImVec4(0.20f, 0.55f, 0.85f, 1.0f);

        m_themes.push_back(t);
    }

    // 🏜️ Desert Storm
    {
        ThemeDefinition t;
        t.id = "desert";
        t.name = "Desert Storm";
        t.emoji = "🏜️";
        t.description = "Sandy beige and burnt orange dry heat theme";

        auto& c = t.colors;
        c.windowBg = ImVec4(0.22f, 0.18f, 0.14f, 1.0f);
        c.childBg = ImVec4(0.24f, 0.20f, 0.16f, 1.0f);
        c.popupBg = ImVec4(0.23f, 0.19f, 0.15f, 0.98f);
        c.border = ImVec4(0.60f, 0.45f, 0.25f, 0.4f);
        c.frameBg = ImVec4(0.30f, 0.25f, 0.18f, 1.0f);
        c.frameBgHovered = ImVec4(0.38f, 0.30f, 0.22f, 1.0f);
        c.frameBgActive = ImVec4(0.75f, 0.50f, 0.20f, 0.5f);
        c.titleBg = ImVec4(0.18f, 0.14f, 0.10f, 1.0f);
        c.titleBgActive = ImVec4(0.55f, 0.38f, 0.15f, 0.8f);
        c.menuBarBg = ImVec4(0.20f, 0.16f, 0.12f, 1.0f);
        c.scrollbarBg = ImVec4(0.18f, 0.14f, 0.10f, 1.0f);
        c.scrollbarGrab = ImVec4(0.60f, 0.45f, 0.25f, 0.6f);
        c.checkMark = ImVec4(0.92f, 0.65f, 0.20f, 1.0f);
        c.sliderGrab = ImVec4(0.85f, 0.55f, 0.18f, 1.0f);
        c.button = ImVec4(0.45f, 0.32f, 0.18f, 0.7f);
        c.buttonHovered = ImVec4(0.60f, 0.42f, 0.22f, 0.8f);
        c.buttonActive = ImVec4(0.80f, 0.55f, 0.20f, 0.9f);
        c.header = ImVec4(0.38f, 0.28f, 0.16f, 0.5f);
        c.headerHovered = ImVec4(0.50f, 0.36f, 0.18f, 0.6f);
        c.headerActive = ImVec4(0.65f, 0.45f, 0.20f, 0.7f);
        c.separator = ImVec4(0.60f, 0.45f, 0.25f, 0.3f);
        c.tab = ImVec4(0.25f, 0.20f, 0.15f, 1.0f);
        c.tabHovered = ImVec4(0.50f, 0.36f, 0.18f, 0.6f);
        c.tabActive = ImVec4(0.42f, 0.30f, 0.18f, 0.8f);
        c.text = ImVec4(0.95f, 0.90f, 0.80f, 1.0f);
        c.textDisabled = ImVec4(0.55f, 0.48f, 0.38f, 1.0f);
        c.plotLines = ImVec4(0.92f, 0.65f, 0.20f, 1.0f);
        c.plotHistogram = ImVec4(0.75f, 0.50f, 0.18f, 1.0f);
        c.tableHeaderBg = ImVec4(0.30f, 0.25f, 0.18f, 1.0f);
        c.tableRowBg = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
        c.tableRowBgAlt = ImVec4(0.40f, 0.30f, 0.15f, 0.05f);
        c.success = ImVec4(0.40f, 0.72f, 0.30f, 1.0f);
        c.warning = ImVec4(0.92f, 0.65f, 0.15f, 1.0f);
        c.error = ImVec4(0.85f, 0.25f, 0.18f, 1.0f);
        c.info = ImVec4(0.50f, 0.65f, 0.80f, 1.0f);
        c.accent = ImVec4(0.92f, 0.65f, 0.20f, 1.0f);

        m_themes.push_back(t);
    }

    // 🔴 Blood Moon
    {
        ThemeDefinition t;
        t.id = "blood_moon";
        t.name = "Blood Moon";
        t.emoji = "🔴";
        t.description = "Dark crimson and deep red high drama theme";

        auto& c = t.colors;
        c.windowBg = ImVec4(0.08f, 0.04f, 0.05f, 1.0f);
        c.childBg = ImVec4(0.10f, 0.05f, 0.06f, 1.0f);
        c.popupBg = ImVec4(0.09f, 0.05f, 0.06f, 0.98f);
        c.border = ImVec4(0.65f, 0.12f, 0.15f, 0.3f);
        c.frameBg = ImVec4(0.16f, 0.07f, 0.08f, 1.0f);
        c.frameBgHovered = ImVec4(0.25f, 0.10f, 0.12f, 1.0f);
        c.frameBgActive = ImVec4(0.60f, 0.12f, 0.15f, 0.5f);
        c.titleBg = ImVec4(0.06f, 0.03f, 0.03f, 1.0f);
        c.titleBgActive = ImVec4(0.50f, 0.08f, 0.12f, 0.8f);
        c.menuBarBg = ImVec4(0.07f, 0.03f, 0.04f, 1.0f);
        c.scrollbarBg = ImVec4(0.06f, 0.03f, 0.03f, 1.0f);
        c.scrollbarGrab = ImVec4(0.55f, 0.10f, 0.14f, 0.5f);
        c.checkMark = ImVec4(0.90f, 0.20f, 0.25f, 1.0f);
        c.sliderGrab = ImVec4(0.80f, 0.15f, 0.20f, 1.0f);
        c.button = ImVec4(0.45f, 0.08f, 0.12f, 0.7f);
        c.buttonHovered = ImVec4(0.60f, 0.12f, 0.18f, 0.8f);
        c.buttonActive = ImVec4(0.80f, 0.18f, 0.22f, 0.9f);
        c.header = ImVec4(0.38f, 0.07f, 0.10f, 0.5f);
        c.headerHovered = ImVec4(0.52f, 0.10f, 0.14f, 0.6f);
        c.headerActive = ImVec4(0.65f, 0.12f, 0.18f, 0.7f);
        c.separator = ImVec4(0.55f, 0.10f, 0.14f, 0.3f);
        c.tab = ImVec4(0.12f, 0.05f, 0.06f, 1.0f);
        c.tabHovered = ImVec4(0.50f, 0.08f, 0.12f, 0.6f);
        c.tabActive = ImVec4(0.42f, 0.07f, 0.10f, 0.8f);
        c.text = ImVec4(0.95f, 0.88f, 0.88f, 1.0f);
        c.textDisabled = ImVec4(0.50f, 0.35f, 0.38f, 1.0f);
        c.plotLines = ImVec4(0.90f, 0.20f, 0.25f, 1.0f);
        c.plotHistogram = ImVec4(0.70f, 0.12f, 0.18f, 1.0f);
        c.tableHeaderBg = ImVec4(0.16f, 0.07f, 0.08f, 1.0f);
        c.tableRowBg = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
        c.tableRowBgAlt = ImVec4(0.30f, 0.05f, 0.08f, 0.06f);
        c.success = ImVec4(0.25f, 0.75f, 0.35f, 1.0f);
        c.warning = ImVec4(0.95f, 0.70f, 0.15f, 1.0f);
        c.error = ImVec4(1.0f, 0.15f, 0.20f, 1.0f);
        c.info = ImVec4(0.50f, 0.60f, 0.85f, 1.0f);
        c.accent = ImVec4(0.90f, 0.20f, 0.25f, 1.0f);

        m_themes.push_back(t);
    }

    // 🤖 Machine
    {
        ThemeDefinition t;
        t.id = "machine";
        t.name = "Machine";
        t.emoji = "🤖";
        t.description = "Cold chrome silver and white industrial robot theme";

        auto& c = t.colors;
        c.windowBg = ImVec4(0.18f, 0.19f, 0.20f, 1.0f);
        c.childBg = ImVec4(0.20f, 0.21f, 0.22f, 1.0f);
        c.popupBg = ImVec4(0.19f, 0.20f, 0.21f, 0.98f);
        c.border = ImVec4(0.55f, 0.58f, 0.60f, 0.4f);
        c.frameBg = ImVec4(0.26f, 0.27f, 0.28f, 1.0f);
        c.frameBgHovered = ImVec4(0.34f, 0.36f, 0.38f, 1.0f);
        c.frameBgActive = ImVec4(0.45f, 0.48f, 0.50f, 1.0f);
        c.titleBg = ImVec4(0.14f, 0.15f, 0.16f, 1.0f);
        c.titleBgActive = ImVec4(0.35f, 0.38f, 0.40f, 1.0f);
        c.menuBarBg = ImVec4(0.16f, 0.17f, 0.18f, 1.0f);
        c.scrollbarBg = ImVec4(0.14f, 0.15f, 0.16f, 1.0f);
        c.scrollbarGrab = ImVec4(0.50f, 0.52f, 0.55f, 1.0f);
        c.checkMark = ImVec4(0.85f, 0.88f, 0.90f, 1.0f);
        c.sliderGrab = ImVec4(0.70f, 0.73f, 0.76f, 1.0f);
        c.button = ImVec4(0.32f, 0.34f, 0.36f, 1.0f);
        c.buttonHovered = ImVec4(0.42f, 0.45f, 0.48f, 1.0f);
        c.buttonActive = ImVec4(0.55f, 0.58f, 0.62f, 1.0f);
        c.header = ImVec4(0.28f, 0.30f, 0.32f, 1.0f);
        c.headerHovered = ImVec4(0.38f, 0.40f, 0.43f, 1.0f);
        c.headerActive = ImVec4(0.48f, 0.52f, 0.55f, 1.0f);
        c.separator = ImVec4(0.50f, 0.52f, 0.55f, 0.3f);
        c.tab = ImVec4(0.22f, 0.23f, 0.24f, 1.0f);
        c.tabHovered = ImVec4(0.38f, 0.40f, 0.43f, 1.0f);
        c.tabActive = ImVec4(0.30f, 0.32f, 0.34f, 1.0f);
        c.text = ImVec4(0.92f, 0.93f, 0.95f, 1.0f);
        c.textDisabled = ImVec4(0.50f, 0.52f, 0.54f, 1.0f);
        c.plotLines = ImVec4(0.80f, 0.82f, 0.85f, 1.0f);
        c.plotHistogram = ImVec4(0.60f, 0.62f, 0.65f, 1.0f);
        c.tableHeaderBg = ImVec4(0.26f, 0.27f, 0.28f, 1.0f);
        c.tableRowBg = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
        c.tableRowBgAlt = ImVec4(1.0f, 1.0f, 1.0f, 0.02f);
        c.success = ImVec4(0.35f, 0.80f, 0.45f, 1.0f);
        c.warning = ImVec4(0.90f, 0.75f, 0.20f, 1.0f);
        c.error = ImVec4(0.85f, 0.28f, 0.25f, 1.0f);
        c.info = ImVec4(0.45f, 0.65f, 0.90f, 1.0f);
        c.accent = ImVec4(0.80f, 0.82f, 0.85f, 1.0f);

        m_themes.push_back(t);
    }
}

void ThemeManager::apply(const std::string& themeId) {
    for (size_t i = 0; i < m_themes.size(); ++i) {
        if (m_themes[i].id == themeId) {
            m_currentThemeIndex = static_cast<int>(i);
            applyToImGui(m_themes[i]);
            return;
        }
    }
}

void ThemeManager::applyToImGui(const ThemeDefinition& theme) {
    ImGuiStyle& style = ImGui::GetStyle();
    const auto& c = theme.colors;

    style.WindowRounding = theme.windowRounding;
    style.FrameRounding = theme.frameRounding;
    style.GrabRounding = theme.grabRounding;
    style.TabRounding = theme.tabRounding;
    style.ChildRounding = theme.windowRounding * 0.5f;
    style.PopupRounding = theme.windowRounding * 0.5f;
    style.ScrollbarRounding = theme.frameRounding;

    style.WindowPadding = ImVec2(10, 10);
    style.FramePadding = ImVec2(8, 4);
    style.ItemSpacing = ImVec2(8, 6);
    style.ItemInnerSpacing = ImVec2(6, 4);
    style.ScrollbarSize = 14.0f;
    style.GrabMinSize = 12.0f;

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = c.windowBg;
    colors[ImGuiCol_ChildBg] = c.childBg;
    colors[ImGuiCol_PopupBg] = c.popupBg;
    colors[ImGuiCol_Border] = c.border;
    colors[ImGuiCol_BorderShadow] = ImVec4(0, 0, 0, 0);
    colors[ImGuiCol_FrameBg] = c.frameBg;
    colors[ImGuiCol_FrameBgHovered] = c.frameBgHovered;
    colors[ImGuiCol_FrameBgActive] = c.frameBgActive;
    colors[ImGuiCol_TitleBg] = c.titleBg;
    colors[ImGuiCol_TitleBgActive] = c.titleBgActive;
    colors[ImGuiCol_TitleBgCollapsed] = c.titleBg;
    colors[ImGuiCol_MenuBarBg] = c.menuBarBg;
    colors[ImGuiCol_ScrollbarBg] = c.scrollbarBg;
    colors[ImGuiCol_ScrollbarGrab] = c.scrollbarGrab;
    colors[ImGuiCol_ScrollbarGrabHovered] = c.scrollbarGrab;
    colors[ImGuiCol_ScrollbarGrabActive] = c.scrollbarGrab;
    colors[ImGuiCol_CheckMark] = c.checkMark;
    colors[ImGuiCol_SliderGrab] = c.sliderGrab;
    colors[ImGuiCol_SliderGrabActive] = c.sliderGrab;
    colors[ImGuiCol_Button] = c.button;
    colors[ImGuiCol_ButtonHovered] = c.buttonHovered;
    colors[ImGuiCol_ButtonActive] = c.buttonActive;
    colors[ImGuiCol_Header] = c.header;
    colors[ImGuiCol_HeaderHovered] = c.headerHovered;
    colors[ImGuiCol_HeaderActive] = c.headerActive;
    colors[ImGuiCol_Separator] = c.separator;
    colors[ImGuiCol_SeparatorHovered] = c.separator;
    colors[ImGuiCol_SeparatorActive] = c.separator;
    colors[ImGuiCol_ResizeGrip] = c.button;
    colors[ImGuiCol_ResizeGripHovered] = c.buttonHovered;
    colors[ImGuiCol_ResizeGripActive] = c.buttonActive;
    colors[ImGuiCol_Tab] = c.tab;
    colors[ImGuiCol_TabHovered] = c.tabHovered;
    colors[ImGuiCol_TabActive] = c.tabActive;
    colors[ImGuiCol_TabUnfocused] = c.tab;
    colors[ImGuiCol_TabUnfocusedActive] = c.tabActive;
    colors[ImGuiCol_Text] = c.text;
    colors[ImGuiCol_TextDisabled] = c.textDisabled;
    colors[ImGuiCol_PlotLines] = c.plotLines;
    colors[ImGuiCol_PlotLinesHovered] = c.plotLines;
    colors[ImGuiCol_PlotHistogram] = c.plotHistogram;
    colors[ImGuiCol_PlotHistogramHovered] = c.plotHistogram;
    colors[ImGuiCol_TableHeaderBg] = c.tableHeaderBg;
    colors[ImGuiCol_TableBorderStrong] = c.border;
    colors[ImGuiCol_TableBorderLight] = ImVec4(c.border.x, c.border.y, c.border.z, c.border.w * 0.5f);
    colors[ImGuiCol_TableRowBg] = c.tableRowBg;
    colors[ImGuiCol_TableRowBgAlt] = c.tableRowBgAlt;
    colors[ImGuiCol_TextSelectedBg] = ImVec4(c.accent.x, c.accent.y, c.accent.z, 0.35f);
    colors[ImGuiCol_DragDropTarget] = c.accent;
    colors[ImGuiCol_NavHighlight] = c.accent;
    colors[ImGuiCol_DockingPreview] = ImVec4(c.accent.x, c.accent.y, c.accent.z, 0.7f);
    colors[ImGuiCol_DockingEmptyBg] = ImVec4(c.windowBg.x * 0.5f, c.windowBg.y * 0.5f, c.windowBg.z * 0.5f, 1.0f);
}

const std::vector<ThemeDefinition>& ThemeManager::getThemes() const {
    return m_themes;
}

const ThemeDefinition& ThemeManager::getCurrentTheme() const {
    return m_themes[m_currentThemeIndex];
}

std::string ThemeManager::getCurrentThemeId() const {
    return m_themes[m_currentThemeIndex].id;
}

const ThemeColors& ThemeManager::colors() const {
    return m_themes[m_currentThemeIndex].colors;
}

} // namespace PSM
