#include "ui/SciFiTheme.h"
#include <imgui.h>

namespace terrafirma {

void SciFiTheme::apply() {
    ImGuiStyle& style = ImGui::GetStyle();
    
    // Colors - Sci-Fi theme
    ImVec4* colors = style.Colors;
    
    // Background colors
    colors[ImGuiCol_WindowBg] = ImVec4(0.0f, 0.04f, 0.08f, 0.94f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.0f, 0.06f, 0.1f, 0.8f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.0f, 0.04f, 0.08f, 0.94f);
    
    // Borders
    colors[ImGuiCol_Border] = ImVec4(0.0f, 0.6f, 0.8f, 0.5f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    
    // Frame backgrounds
    colors[ImGuiCol_FrameBg] = ImVec4(0.0f, 0.1f, 0.15f, 0.8f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.0f, 0.2f, 0.3f, 0.8f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.0f, 0.3f, 0.4f, 0.8f);
    
    // Title bar
    colors[ImGuiCol_TitleBg] = ImVec4(0.0f, 0.1f, 0.15f, 1.0f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.0f, 0.15f, 0.25f, 1.0f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0f, 0.1f, 0.15f, 0.5f);
    
    // Menu bar
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.0f, 0.08f, 0.12f, 1.0f);
    
    // Scrollbar
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.0f, 0.05f, 0.1f, 0.8f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.0f, 0.4f, 0.6f, 0.8f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.0f, 0.5f, 0.7f, 0.8f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.0f, 0.6f, 0.8f, 1.0f);
    
    // Check mark
    colors[ImGuiCol_CheckMark] = ImVec4(0.0f, 1.0f, 1.0f, 1.0f);
    
    // Slider
    colors[ImGuiCol_SliderGrab] = ImVec4(0.0f, 0.6f, 0.8f, 1.0f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.0f, 0.8f, 1.0f, 1.0f);
    
    // Buttons
    colors[ImGuiCol_Button] = ImVec4(0.0f, 0.2f, 0.3f, 0.8f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.0f, 0.3f, 0.5f, 1.0f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.0f, 0.5f, 0.7f, 1.0f);
    
    // Headers
    colors[ImGuiCol_Header] = ImVec4(0.0f, 0.2f, 0.3f, 0.8f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.0f, 0.3f, 0.5f, 0.8f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.0f, 0.4f, 0.6f, 1.0f);
    
    // Separator
    colors[ImGuiCol_Separator] = ImVec4(0.0f, 0.4f, 0.6f, 0.5f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.0f, 0.6f, 0.8f, 0.8f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.0f, 0.8f, 1.0f, 1.0f);
    
    // Resize grip
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.0f, 0.4f, 0.6f, 0.4f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.0f, 0.6f, 0.8f, 0.6f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.0f, 0.8f, 1.0f, 0.8f);
    
    // Tab
    colors[ImGuiCol_Tab] = ImVec4(0.0f, 0.15f, 0.25f, 0.8f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.0f, 0.3f, 0.5f, 0.8f);
    colors[ImGuiCol_TabActive] = ImVec4(0.0f, 0.25f, 0.4f, 1.0f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.0f, 0.1f, 0.15f, 0.8f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.0f, 0.15f, 0.25f, 1.0f);
    
    // Text
    colors[ImGuiCol_Text] = ImVec4(0.8f, 1.0f, 1.0f, 1.0f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.3f, 0.5f, 0.6f, 1.0f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.0f, 0.4f, 0.6f, 0.5f);
    
    // Nav highlight
    colors[ImGuiCol_NavHighlight] = ImVec4(0.0f, 0.8f, 1.0f, 1.0f);
    
    // Style settings
    style.WindowRounding = 4.0f;
    style.FrameRounding = 2.0f;
    style.GrabRounding = 2.0f;
    style.ScrollbarRounding = 2.0f;
    style.TabRounding = 2.0f;
    
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;
    
    style.WindowPadding = ImVec2(10, 10);
    style.FramePadding = ImVec2(6, 4);
    style.ItemSpacing = ImVec2(8, 6);
    style.ItemInnerSpacing = ImVec2(6, 4);
    
    style.IndentSpacing = 20.0f;
    style.ScrollbarSize = 14.0f;
    style.GrabMinSize = 12.0f;
}

} // namespace terrafirma

