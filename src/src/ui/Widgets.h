#pragma once

#include <string>
#include <imgui.h>
#include "ui/Theme.h"

namespace PSM {
namespace Widgets {

// Status badge
void StatusBadge(const char* label, const ImVec4& color, const ImVec4& textColor = ImVec4(1,1,1,1));

// Progress bar with text
void ProgressBarLabeled(float fraction, const ImVec2& size, const char* overlay = nullptr);

// Toggle switch
bool ToggleSwitch(const char* label, bool* v);

// Color-coded text
void TextColored(const char* text, const ImVec4& color);
void TextSuccess(const char* text);
void TextWarning(const char* text);
void TextError(const char* text);
void TextInfo(const char* text);

// Stat card for dashboard
void StatCard(const char* label, const char* value, const char* icon = nullptr,
              const ImVec4& color = ImVec4(0.4f, 0.65f, 0.95f, 1.0f));

// Search/filter input
bool SearchInput(const char* label, char* buf, size_t bufSize, const char* hint = "Search...");

// Confirmation dialog
bool ConfirmDialog(const char* title, const char* message, bool* open);

// Password input with show/hide toggle
bool PasswordInput(const char* label, char* buf, size_t bufSize, bool* showPassword);

// Tooltip helper
void HelpMarker(const char* desc);

// Separator with text
void SeparatorText(const char* text);

// Tab button for sidebar
bool SidebarButton(const char* label, const char* icon, bool selected, const ImVec2& size);

// FPS graph
void FPSGraph(const float* values, int count, float minVal, float maxVal,
              const ImVec2& size = ImVec2(0, 80));

// Player list item
bool PlayerListItem(const char* name, const char* uid, bool online,
                    int level = 0, int suspicionScore = 0);

// Notification toast
void ShowNotification(const char* title, const char* message, const ImVec4& color, float duration = 3.0f);

} // namespace Widgets
} // namespace PSM
