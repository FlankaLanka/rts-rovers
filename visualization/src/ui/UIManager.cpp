#include "ui/UIManager.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <cstdio>
#include <iostream>

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

void UIManager::renderMainUI(DataManager* dataManager, UDPReceiver* udpReceiver,
                             int& selectedRover, bool& followRover,
                             RenderSettings& settings, float fps, Camera* camera,
                             TerrainOperationManager* opManager,
                             std::array<bool, NUM_ROVERS>* manualControl,
                             std::array<bool, NUM_ROVERS>* rtsMode,
                             std::array<bool, NUM_ROVERS>* wayMode) {
    renderRoverPanel(dataManager, selectedRover, manualControl);
    renderStatusPanel(dataManager, udpReceiver, selectedRover, followRover, camera, opManager, manualControl, rtsMode, wayMode);
    renderSettingsPanel(settings);
    renderSystemPanel(dataManager, fps);
    
    if (opManager) {
        renderOperationPanel(opManager, selectedRover);
    }
}

void UIManager::renderRoverPanel(DataManager* dataManager, int& selectedRover,
                                  std::array<bool, NUM_ROVERS>* manualControl) {
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(250, 450), ImGuiCond_FirstUseEver);
    
    ImGui::Begin("ROVER FLEET", nullptr, ImGuiWindowFlags_NoCollapse);
    
    for (int i = 0; i < NUM_ROVERS; i++) {
        auto& rover = dataManager->getRover(i);
        bool engineRunning = dataManager->isRoverEngineRunning(i);
        bool isManualControl = manualControl && (*manualControl)[i];
        
        ImGui::PushID(i);
        
        // Selection indicator
        bool isSelected = (selectedRover == i);
        if (isSelected) {
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0f, 0.2f, 0.3f, 0.8f));
        }
        
        ImGui::BeginChild(("rover" + std::to_string(i)).c_str(), ImVec2(-1, 80), true);
        
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
        
        // Engine/Manual/RTS/WAY status indicator
        ImGui::SameLine();
        if (!rover.online) {
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "[--]");
        } else if (isManualControl) {
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "[CTRL]");  // Yellow for manual control
        } else if (engineRunning) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), "[ON]");
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.0f, 1.0f), "[OFF]");
        }
        // Note: RTS/WAY status is shown in status panel since we don't have access to those arrays here
        
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

void UIManager::renderStatusPanel(DataManager* dataManager, UDPReceiver* udpReceiver,
                                   int selectedRover, bool& followRover, Camera* camera,
                                   TerrainOperationManager* opManager,
                                   std::array<bool, NUM_ROVERS>* manualControl,
                                   std::array<bool, NUM_ROVERS>* rtsMode,
                                   std::array<bool, NUM_ROVERS>* wayMode) {
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 310, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(300, 600), ImGuiCond_FirstUseEver);  // Increased height for new buttons
    
    auto& rover = dataManager->getRover(selectedRover);
    bool engineRunning = dataManager->isRoverEngineRunning(selectedRover);
    bool isManualControl = manualControl && (*manualControl)[selectedRover];
    bool isRtsMode = rtsMode && (*rtsMode)[selectedRover];
    bool isWayMode = wayMode && (*wayMode)[selectedRover];
    
    char title[64];
    snprintf(title, sizeof(title), "ROVER %02d STATUS", selectedRover + 1);
    
    ImGui::Begin(title, nullptr, ImGuiWindowFlags_NoCollapse);
    
    // Engine/Control status indicator
    ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "STATUS");
    ImGui::Separator();
    if (isRtsMode) {
        ImGui::TextColored(ImVec4(0.6f, 0.3f, 1.0f, 1.0f), "RTS MODE - Click terrain to move");
    } else if (isWayMode) {
        ImGui::TextColored(ImVec4(0.3f, 0.8f, 1.0f, 1.0f), "WAYPOINT MODE - Auto exploring");
    } else if (isManualControl) {
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "MANUAL CONTROL - WASD to drive");
    } else if (engineRunning) {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), "ENGINE: RUNNING");
    } else {
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.0f, 1.0f), "ENGINE: STOPPED");
    }
    
    ImGui::Spacing();
    
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
    
    // Button states / Controls
    ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "CONTROLS");
    ImGui::Separator();
    
    // Get operation state for this rover
    TerrainOperation* op = opManager ? &opManager->getOperation(selectedRover) : nullptr;
    OperationState opState = op ? op->getState() : OperationState::IDLE;
    
    for (int i = 0; i < 4; i++) {
        bool isOn = (rover.buttonStates >> i) & 1;
        
        // Button styling based on type and state
        if (i == 0) {
            // Button 0 (engine): Green for ON, Red for OFF
            if (isOn) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.6f, 0.2f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.7f, 0.3f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 0.8f, 0.4f, 1.0f));
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.1f, 0.1f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
            }
        } else if (i == 1) {
            // Button 1 (DIG): Blue colors
            bool isDigging = op && op->getType() == OperationType::DIG && opState != OperationState::IDLE;
            if (isDigging) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.4f, 0.8f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.5f, 0.9f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.6f, 1.0f, 1.0f));
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.2f, 0.4f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.3f, 0.5f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.3f, 0.4f, 0.6f, 1.0f));
            }
        } else if (i == 2) {
            // Button 2 (PILE): Brown/Orange colors
            bool isPiling = op && op->getType() == OperationType::PILE && opState != OperationState::IDLE;
            if (isPiling) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.5f, 0.2f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.6f, 0.3f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.7f, 0.4f, 1.0f));
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.4f, 0.25f, 0.1f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.5f, 0.35f, 0.2f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.45f, 0.3f, 1.0f));
            }
        } else {
            // Button 3 (CTRL): Yellow/Gold for manual control
            if (isManualControl) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.6f, 0.0f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.7f, 0.1f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.8f, 0.2f, 1.0f));
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.25f, 0.1f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.35f, 0.15f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 0.45f, 0.2f, 1.0f));
            }
        }
        
        // Button labels
        char label[16];
        if (i == 0) {
            snprintf(label, sizeof(label), isOn ? "ON" : "OFF");
        } else if (i == 1) {
            snprintf(label, sizeof(label), "DIG");
        } else if (i == 2) {
            snprintf(label, sizeof(label), "PILE");
        } else if (i == 3) {
            snprintf(label, sizeof(label), "CTRL");
        } else {
            snprintf(label, sizeof(label), "BTN %d", i);
        }
        
        // Button click handling
        if (ImGui::Button(label, ImVec2(60, 40))) {
            if (i == 0) {
                // Toggle engine
                uint8_t newState = rover.buttonStates ^ (1 << 0);
                udpReceiver->sendCommand(selectedRover + 1, newState);
            } else if (i == 1 && op) {
                // DIG button
                if (opState == OperationState::IDLE && !isManualControl) {
                    // Start drawing circle for dig
                    op->startDrawing(OperationType::DIG);
                    m_isDrawingCircle = true;
                } else if (op->getType() == OperationType::DIG) {
                    // Cancel current dig operation
                    op->cancel();
                    m_isDrawingCircle = false;
                }
            } else if (i == 2 && op) {
                // PILE button
                if (opState == OperationState::IDLE && !isManualControl) {
                    // Start drawing circle for pile
                    op->startDrawing(OperationType::PILE);
                    m_isDrawingCircle = true;
                } else if (op->getType() == OperationType::PILE) {
                    // Cancel current pile operation
                    op->cancel();
                    m_isDrawingCircle = false;
                }
            } else if (i == 3 && manualControl) {
                // CTRL button - toggle manual control
                (*manualControl)[selectedRover] = !(*manualControl)[selectedRover];
                if ((*manualControl)[selectedRover]) {
                    std::cout << "Manual control ENABLED for rover " << (selectedRover + 1) << "\n";
                    // Disable follow mode when entering manual control
                    followRover = false;
                } else {
                    std::cout << "Manual control DISABLED for rover " << (selectedRover + 1) << "\n";
                }
            }
        }
        
        // Tooltips
        if (ImGui::IsItemHovered()) {
            if (i == 0) {
                ImGui::SetTooltip("Click to %s engine", isOn ? "stop" : "start");
            } else if (i == 1) {
                if (opState == OperationState::IDLE) {
                    ImGui::SetTooltip("Click to start DIG operation");
                } else if (op && op->getType() == OperationType::DIG) {
                    ImGui::SetTooltip("Click to cancel DIG operation");
                }
            } else if (i == 2) {
                if (opState == OperationState::IDLE) {
                    ImGui::SetTooltip("Click to start PILE operation");
                } else if (op && op->getType() == OperationType::PILE) {
                    ImGui::SetTooltip("Click to cancel PILE operation");
                }
            } else if (i == 3) {
                ImGui::SetTooltip("Click to %s manual control (WASD to drive)", isManualControl ? "disable" : "enable");
            }
        }
        
        ImGui::PopStyleColor(3);
        
        if (i < 3) ImGui::SameLine();
    }
    
    ImGui::Spacing();
    
    // RTS and WAY buttons (new row)
    if (rtsMode && wayMode) {
        // RTS Button (Purple)
        if (isRtsMode) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.3f, 1.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.4f, 1.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.8f, 0.5f, 1.0f, 1.0f));
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.15f, 0.4f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.35f, 0.2f, 0.5f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.45f, 0.3f, 0.6f, 1.0f));
        }
        
        if (ImGui::Button("RTS", ImVec2(60, 40))) {
            // Toggle RTS mode - disable other modes
            bool newState = !(*rtsMode)[selectedRover];
            (*rtsMode)[selectedRover] = newState;
            if (newState) {
                (*wayMode)[selectedRover] = false;
                if (manualControl) (*manualControl)[selectedRover] = false;
                followRover = false;
                std::cout << "RTS mode ENABLED for rover " << (selectedRover + 1) << " - click terrain to set destination\n";
            } else {
                std::cout << "RTS mode DISABLED for rover " << (selectedRover + 1) << "\n";
            }
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Click terrain to pathfind to that location");
        }
        ImGui::PopStyleColor(3);
        
        ImGui::SameLine();
        
        // WAY Button (Cyan)
        if (isWayMode) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.9f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.8f, 1.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.9f, 1.0f, 1.0f));
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.3f, 0.4f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.15f, 0.4f, 0.5f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.5f, 0.6f, 1.0f));
        }
        
        if (ImGui::Button("WAY", ImVec2(60, 40))) {
            // Toggle WAY mode - disable other modes
            bool newState = !(*wayMode)[selectedRover];
            (*wayMode)[selectedRover] = newState;
            if (newState) {
                (*rtsMode)[selectedRover] = false;
                if (manualControl) (*manualControl)[selectedRover] = false;
                followRover = false;
                std::cout << "WAY mode ENABLED for rover " << (selectedRover + 1) << " - auto waypoint exploration\n";
            } else {
                std::cout << "WAY mode DISABLED for rover " << (selectedRover + 1) << "\n";
            }
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Auto-spawn waypoints for autonomous exploration");
        }
        ImGui::PopStyleColor(3);
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

void UIManager::renderOperationPanel(TerrainOperationManager* opManager, int selectedRover) {
    auto& op = opManager->getOperation(selectedRover);
    OperationState state = op.getState();
    
    // Only show panel if operation is active
    if (state == OperationState::IDLE) {
        return;
    }
    
    // Center the panel on screen
    ImVec2 windowSize(300, 150);
    ImVec2 screenSize = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowPos(ImVec2((screenSize.x - windowSize.x) * 0.5f, 100), ImGuiCond_Always);
    ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);
    
    const char* opTypeName = (op.getType() == OperationType::DIG) ? "DIG" : "PILE";
    char title[64];
    snprintf(title, sizeof(title), "%s OPERATION", opTypeName);
    
    ImGui::Begin(title, nullptr, 
                 ImGuiWindowFlags_NoCollapse | 
                 ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoMove);
    
    switch (state) {
        case OperationState::DRAWING:
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "DRAWING CIRCLE");
            ImGui::Separator();
            ImGui::Text("Click and drag on terrain to");
            ImGui::Text("draw the %s area.", op.getType() == OperationType::DIG ? "dig" : "pile");
            ImGui::Spacing();
            if (op.getRadius() > 0) {
                ImGui::Text("Radius: %.1f m", op.getRadius());
            } else {
                ImGui::Text("Click on terrain to start...");
            }
            break;
            
        case OperationState::CONFIRMING:
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "CONFIRM AREA");
            ImGui::Separator();
            ImGui::Text("Center: (%.1f, %.1f)", op.getCenter().x, op.getCenter().y);
            ImGui::Text("Radius: %.1f m", op.getRadius());
            ImGui::Spacing();
            
            // Confirm and Redo buttons
            if (ImGui::Button("CONFIRM", ImVec2(120, 40))) {
                op.confirm();
                m_isDrawingCircle = false;
            }
            ImGui::SameLine();
            if (ImGui::Button("REDO", ImVec2(120, 40))) {
                op.redo();
            }
            break;
            
        case OperationState::MOVING:
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), "MOVING TO SITE");
            ImGui::Separator();
            ImGui::Text("Rover moving to %s site...", op.getType() == OperationType::DIG ? "dig" : "pile");
            ImGui::Spacing();
            if (ImGui::Button("CANCEL", ImVec2(-1, 40))) {
                op.cancel();
            }
            break;
            
        case OperationState::OPERATING:
            {
                const char* actionName = (op.getType() == OperationType::DIG) ? "DIGGING" : "PILING";
                ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "%s...", actionName);
                ImGui::Separator();
                
                float progress = std::abs(op.getCurrentDepth()) / TerrainOperation::MAX_DEPTH;
                ImGui::ProgressBar(progress, ImVec2(-1, 20));
                ImGui::Text("Depth: %.2f / %.1f m", std::abs(op.getCurrentDepth()), TerrainOperation::MAX_DEPTH);
                ImGui::Spacing();
                
                if (ImGui::Button("STOP", ImVec2(-1, 40))) {
                    op.cancel();
                }
            }
            break;
            
        default:
            break;
    }
    
    ImGui::End();
}

void UIManager::renderSettingsPanel(RenderSettings& settings) {
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 310, 520), ImGuiCond_FirstUseEver);
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
