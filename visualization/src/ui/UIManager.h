#pragma once

#include "common.h"
#include "ui/SciFiTheme.h"
#include "data/DataManager.h"
#include "render/Camera.h"
#include "network/UDPReceiver.h"
#include "terrain/TerrainOperation.h"

struct GLFWwindow;

namespace terrafirma {

class UIManager {
public:
    UIManager(GLFWwindow* window);
    ~UIManager();

    void begin();
    void end();
    
    void renderMainUI(DataManager* dataManager, UDPReceiver* udpReceiver,
                      int& selectedRover, bool& followRover,
                      RenderSettings& settings, float fps, Camera* camera,
                      TerrainOperationManager* opManager,
                      std::array<bool, NUM_ROVERS>* manualControl,
                      std::array<bool, NUM_ROVERS>* rtsMode = nullptr,
                      std::array<bool, NUM_ROVERS>* wayMode = nullptr);
    
    bool wantCaptureMouse() const;
    bool wantCaptureKeyboard() const;
    
    // Terrain operation UI state
    bool isDrawingCircle() const { return m_isDrawingCircle; }
    void setDrawingCircle(bool drawing) { m_isDrawingCircle = drawing; }

private:
    void renderRoverPanel(DataManager* dataManager, int& selectedRover,
                          std::array<bool, NUM_ROVERS>* manualControl);
    void renderStatusPanel(DataManager* dataManager, UDPReceiver* udpReceiver,
                           int selectedRover, bool& followRover, Camera* camera,
                           TerrainOperationManager* opManager,
                           std::array<bool, NUM_ROVERS>* manualControl,
                           std::array<bool, NUM_ROVERS>* rtsMode,
                           std::array<bool, NUM_ROVERS>* wayMode);
    void renderSettingsPanel(RenderSettings& settings);
    void renderSystemPanel(DataManager* dataManager, float fps);
    void renderOperationPanel(TerrainOperationManager* opManager, int selectedRover);
    
    GLFWwindow* m_window;
    SciFiTheme m_theme;
    
    // Circle drawing state
    bool m_isDrawingCircle = false;
};

} // namespace terrafirma
