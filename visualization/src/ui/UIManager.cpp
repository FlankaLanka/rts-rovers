#include "ui/UIManager.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <cstdio>

namespace terrafirma {

UIManager::UIManager(GLFWwindow* window) : m_window(window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    
    m_theme.apply();
}

UIManager::~UIManager() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void UIManager::begin() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void UIManager::end() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UIManager::renderMainUI(DataManager* dataManager, int& selectedRover, bool& followRover,
                             RenderSettings& settings, float fps, Camera* camera) {
    renderRoverPanel(dataManager, selectedRover);
    renderStatusPanel(dataManager, selectedRover, followRover, camera);
    renderSettingsPanel(settings);
    renderSystemPanel(dataManager, fps);
}

void UIManager::renderRoverPanel(DataManager* dataManager, int& selectedRover) {
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(250, 400), ImGuiCond_FirstUseEver);
    
    ImGui::Begin("ROVER FLEET", nullptr, ImGuiWindowFlags_NoCollapse);
    
    for (int i = 0; i < NUM_ROVERS; i++) {
        auto& rover = dataManager->getRover(i);
        
        ImGui::PushID(i);
        
        // Selection indicator
        bool isSelected = (selectedRover == i);
        if (isSelected) {
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0f, 0.2f, 0.3f, 0.8f));
        }
        
        ImGui::BeginChild(("rover" + std::to_string(i)).c_str(), ImVec2(-1, 70), true);
        
        // Online indicator
        if (rover.online) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "●");
        } else {
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "○");
        }
        ImGui::SameLine();
        
        // Rover name with color
        glm::vec3 color = ROVER_COLORS[i];
        ImGui::TextColored(ImVec4(color.r, color.g, color.b, 1.0f), "ROVER %02d", i + 1);
        
        // Status
        ImGui::Text("Status: %s", rover.online ? "ONLINE" : "OFFLINE");
        
        // Point count
        size_t points = dataManager->getPointCloud(i).getPointCount();
        ImGui::Text("Points: %zu", points);
        
        // Click to select
        if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(0)) {
            selectedRover = i;
        }
        
        ImGui::EndChild();
        
        if (isSelected) {
            ImGui::PopStyleColor();
        }
        
        ImGui::PopID();
    }
    
    ImGui::End();
}

void UIManager::renderStatusPanel(DataManager* dataManager, int selectedRover, bool& followRover, Camera* camera) {
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 310, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(300, 450), ImGuiCond_FirstUseEver);
    
    auto& rover = dataManager->getRover(selectedRover);
    
    char title[64];
    snprintf(title, sizeof(title), "ROVER %02d STATUS", selectedRover + 1);
    
    ImGui::Begin(title, nullptr, ImGuiWindowFlags_NoCollapse);
    
    // Position
    ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "POSITION");
    ImGui::Separator();
    ImGui::Text("X: %.2f m", rover.position.x);
    ImGui::Text("Y: %.2f m", rover.position.y);
    ImGui::Text("Z: %.2f m", rover.position.z);
    
    ImGui::Spacing();
    
    // Orientation
    ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "ORIENTATION");
    ImGui::Separator();
    ImGui::Text("Roll:  %.1f°", rover.rotation.x);
    ImGui::Text("Pitch: %.1f°", rover.rotation.y);
    ImGui::Text("Yaw:   %.1f°", rover.rotation.z);
    
    ImGui::Spacing();
    
    // Button states
    ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "BUTTON STATES");
    ImGui::Separator();
    
    for (int i = 0; i < 4; i++) {
        bool isOn = (rover.buttonStates >> i) & 1;
        
        if (isOn) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.5f, 0.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.6f, 0.2f, 1.0f));
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.1f, 0.15f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.2f, 0.25f, 1.0f));
        }
        
        char label[16];
        snprintf(label, sizeof(label), "BTN %d", i);
        ImGui::Button(label, ImVec2(60, 40));
        
        ImGui::PopStyleColor(2);
        
        if (i < 3) ImGui::SameLine();
    }
    
    ImGui::Spacing();
    ImGui::Spacing();
    
    // Camera controls
    ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "CAMERA");
    ImGui::Separator();
    
    if (ImGui::Checkbox("Follow Rover", &followRover)) {
        // Follow mode toggled
    }
    
    if (ImGui::Button("Go to Rover")) {
        camera->setPosition(rover.position + glm::vec3(0, 30, 50));
        camera->lookAt(rover.position);
    }
    
    ImGui::End();
}

void UIManager::renderSettingsPanel(RenderSettings& settings) {
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 310, 470), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiCond_FirstUseEver);
    
    ImGui::Begin("RENDER OPTIONS", nullptr, ImGuiWindowFlags_NoCollapse);
    
    ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "TERRAIN");
    ImGui::Separator();
    ImGui::Checkbox("Show Terrain", &settings.showTerrain);
    ImGui::Checkbox("Solid", &settings.terrainSolid);
    ImGui::Checkbox("Wireframe", &settings.terrainWireframe);
    ImGui::Checkbox("Height Colors", &settings.terrainHeightColors);
    
    ImGui::Spacing();
    
    ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "POINT CLOUD");
    ImGui::Separator();
    ImGui::Checkbox("Show Points", &settings.showPointCloud);
    ImGui::Checkbox("Height Gradient", &settings.pointCloudHeightColors);
    ImGui::SliderFloat("Point Size", &settings.pointSize, 1.0f, 10.0f);
    
    ImGui::End();
}

void UIManager::renderSystemPanel(DataManager* dataManager, float fps) {
    ImGui::SetNextWindowPos(ImVec2(10, ImGui::GetIO().DisplaySize.y - 80), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(300, 70), ImGuiCond_FirstUseEver);
    
    ImGui::Begin("SYSTEM", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
    
    ImGui::Text("FPS: %.1f", fps);
    ImGui::SameLine(150);
    ImGui::Text("Points: %zu", dataManager->getTotalPointCount());
    
    ImGui::Text("Controls: WASD+Mouse (RMB) | 1-5: Select | F: Follow | F11: Fullscreen");
    
    ImGui::End();
}

bool UIManager::wantCaptureMouse() const {
    return ImGui::GetIO().WantCaptureMouse;
}

bool UIManager::wantCaptureKeyboard() const {
    return ImGui::GetIO().WantCaptureKeyboard;
}

} // namespace terrafirma

