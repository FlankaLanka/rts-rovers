#pragma once

#include "common.h"
#include "ui/SciFiTheme.h"
#include "data/DataManager.h"
#include "render/Camera.h"

struct GLFWwindow;

namespace terrafirma {

class UIManager {
public:
    UIManager(GLFWwindow* window);
    ~UIManager();

    void begin();
    void end();
    
    void renderMainUI(DataManager* dataManager, int& selectedRover, bool& followRover,
                      RenderSettings& settings, float fps, Camera* camera);
    
    bool wantCaptureMouse() const;
    bool wantCaptureKeyboard() const;

private:
    void renderRoverPanel(DataManager* dataManager, int& selectedRover);
    void renderStatusPanel(DataManager* dataManager, int selectedRover, bool& followRover, Camera* camera);
    void renderSettingsPanel(RenderSettings& settings);
    void renderSystemPanel(DataManager* dataManager, float fps);
    
    GLFWwindow* m_window;
    SciFiTheme m_theme;
};

} // namespace terrafirma

