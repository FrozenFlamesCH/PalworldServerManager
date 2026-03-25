#include "ui/Widgets.h"
#include <cstring>
#include <cmath>

namespace PSM {
namespace Widgets {

void StatusBadge(const char* label, const ImVec4& color, const ImVec4& textColor) {
    ImVec2 textSize = ImGui::CalcTextSize(label);
    ImVec2 padding(8, 3);
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImDrawList* dl = ImGui::GetWindowDrawList();

    ImVec2 minP = pos;
    ImVec2 maxP(pos.x + textSize.x + padding.x * 2, pos.y + textSize.y + padding.y * 2);

    dl->AddRectFilled(minP, maxP, ImGui::ColorConvertFloat4ToU32(color), 4.0f);
    dl->AddText(ImVec2(pos.x + padding.x, pos.y + padding.y),
                ImGui::ColorConvertFloat4ToU32(textColor), label);

    ImGui::Dummy(ImVec2(textSize.x + padding.x * 2, textSize.y + padding.y * 2));
}

void ProgressBarLabeled(float fraction, const ImVec2& size, const char* overlay) {
    ImGui::ProgressBar(fraction, size, overlay);
}

bool ToggleSwitch(const char* label, bool* v) {
    ImGui::PushID(label);

    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImDrawList* dl = ImGui::GetWindowDrawList();

    float height = ImGui::GetFrameHeight();
    float width = height * 1.8f;
    float radius = height * 0.5f;

    bool changed = false;
    if (ImGui::InvisibleButton(label, ImVec2(width, height))) {
        *v = !*v;
        changed = true;
    }

    float t = *v ? 1.0f : 0.0f;

    ImU32 bgColor = *v ?
        ImGui::ColorConvertFloat4ToU32(ImVec4(0.3f, 0.7f, 0.4f, 1.0f)) :
        ImGui::ColorConvertFloat4ToU32(ImVec4(0.4f, 0.4f, 0.4f, 1.0f));

    dl->AddRectFilled(pos, ImVec2(pos.x + width, pos.y + height), bgColor, radius);

    float knobX = pos.x + radius + t * (width - height);
    dl->AddCircleFilled(ImVec2(knobX, pos.y + radius),
                        radius - 2.0f, IM_COL32(255, 255, 255, 255));

    ImGui::SameLine();
    ImGui::TextUnformatted(label);

    ImGui::PopID();
    return changed;
}

void TextColored(const char* text, const ImVec4& color) {
    ImGui::TextColored(color, "%s", text);
}

void TextSuccess(const char* text) {
    ImGui::TextColored(ImVec4(0.3f, 0.8f, 0.4f, 1.0f), "%s", text);
}

void TextWarning(const char* text) {
    ImGui::TextColored(ImVec4(0.95f, 0.75f, 0.2f, 1.0f), "%s", text);
}

void TextError(const char* text) {
    ImGui::TextColored(ImVec4(0.9f, 0.3f, 0.25f, 1.0f), "%s", text);
}

void TextInfo(const char* text) {
    ImGui::TextColored(ImVec4(0.4f, 0.65f, 0.95f, 1.0f), "%s", text);
}

void StatCard(const char* label, const char* value, const char* icon, const ImVec4& color) {
    ImGui::BeginGroup();

    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImDrawList* dl = ImGui::GetWindowDrawList();
    float width = ImGui::GetContentRegionAvail().x;
    float height = 70.0f;

    ImVec4 bg = ImGui::GetStyleColorVec4(ImGuiCol_ChildBg);
    dl->AddRectFilled(pos, ImVec2(pos.x + width, pos.y + height),
                      ImGui::ColorConvertFloat4ToU32(bg), 6.0f);

    // Left color bar
    dl->AddRectFilled(pos, ImVec2(pos.x + 4, pos.y + height),
                      ImGui::ColorConvertFloat4ToU32(color), 6.0f);

    ImGui::SetCursorScreenPos(ImVec2(pos.x + 14, pos.y + 8));
    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.65f, 1.0f), "%s", label);

    ImGui::SetCursorScreenPos(ImVec2(pos.x + 14, pos.y + 28));
    ImGui::PushFont(nullptr); // Would use larger font
    ImGui::TextColored(color, "%s", value);
    ImGui::PopFont();

    ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + height + 4));
    ImGui::EndGroup();
}

bool SearchInput(const char* label, char* buf, size_t bufSize, const char* hint) {
    ImGui::PushItemWidth(-1);
    bool changed = ImGui::InputTextWithHint(label, hint, buf, bufSize);
    ImGui::PopItemWidth();
    return changed;
}

bool ConfirmDialog(const char* title, const char* message, bool* open) {
    bool confirmed = false;
    if (*open) {
        ImGui::OpenPopup(title);
    }

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal(title, open, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextWrapped("%s", message);
        ImGui::Separator();

        if (ImGui::Button("Confirm", ImVec2(120, 0))) {
            confirmed = true;
            *open = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            *open = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    return confirmed;
}

bool PasswordInput(const char* label, char* buf, size_t bufSize, bool* showPassword) {
    ImGuiInputTextFlags flags = *showPassword ? 0 : ImGuiInputTextFlags_Password;
    float width = ImGui::CalcItemWidth() - 30;
    ImGui::PushItemWidth(width);
    bool changed = ImGui::InputText(label, buf, bufSize, flags);
    ImGui::PopItemWidth();
    ImGui::SameLine();
    ImGui::PushID(label);
    if (ImGui::SmallButton(*showPassword ? "Hide" : "Show")) {
        *showPassword = !*showPassword;
    }
    ImGui::PopID();
    return changed;
}

void HelpMarker(const char* desc) {
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 25.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

void SeparatorText(const char* text) {
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.72f, 1.0f), "%s", text);
    ImGui::Separator();
    ImGui::Spacing();
}

bool SidebarButton(const char* label, const char* icon, bool selected, const ImVec2& size) {
    if (selected) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
    }

    std::string fullLabel = std::string(icon ? icon : "") + "  " + label;
    bool clicked = ImGui::Button(fullLabel.c_str(), size);

    if (selected) {
        ImGui::PopStyleColor();
    }

    return clicked;
}

void FPSGraph(const float* values, int count, float minVal, float maxVal, const ImVec2& size) {
    if (count <= 0) return;

    char overlay[32];
    snprintf(overlay, sizeof(overlay), "%.1f FPS", values[count - 1]);
    ImGui::PlotLines("##fps", values, count, 0, overlay, minVal, maxVal, size);
}

bool PlayerListItem(const char* name, const char* uid, bool online, int level, int suspicionScore) {
    ImGui::PushID(uid);

    bool clicked = ImGui::Selectable("##player", false, ImGuiSelectableFlags_SpanAllColumns);

    ImGui::SameLine();

    // Online indicator
    ImVec4 statusColor = online ? ImVec4(0.3f, 0.8f, 0.4f, 1.0f) : ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImGui::GetWindowDrawList()->AddCircleFilled(
        ImVec2(pos.x + 5, pos.y + ImGui::GetTextLineHeight() * 0.5f),
        4.0f, ImGui::ColorConvertFloat4ToU32(statusColor));
    ImGui::Dummy(ImVec2(14, 0));
    ImGui::SameLine();

    ImGui::Text("%s", name);
    ImGui::SameLine(200);
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.55f, 1.0f), "Lv.%d", level);

    if (suspicionScore > 0) {
        ImGui::SameLine(260);
        ImVec4 scoreColor = suspicionScore > 50 ?
            ImVec4(0.9f, 0.3f, 0.25f, 1.0f) :
            (suspicionScore > 20 ?
                ImVec4(0.95f, 0.75f, 0.2f, 1.0f) :
                ImVec4(0.5f, 0.5f, 0.55f, 1.0f));
        ImGui::TextColored(scoreColor, "Score: %d", suspicionScore);
    }

    ImGui::PopID();
    return clicked;
}

// Notification system
struct NotificationData {
    std::string title;
    std::string message;
    ImVec4 color;
    float startTime;
    float duration;
};

static std::vector<NotificationData> g_notifications;

void ShowNotification(const char* title, const char* message, const ImVec4& color, float duration) {
    NotificationData n;
    n.title = title;
    n.message = message;
    n.color = color;
    n.startTime = static_cast<float>(ImGui::GetTime());
    n.duration = duration;
    g_notifications.push_back(n);
}

} // namespace Widgets
} // namespace PSM
