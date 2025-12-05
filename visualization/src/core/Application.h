#pragma once

#include "common.h"
#include "core/Timer.h"
#include "network/UDPReceiver.h"
#include "data/DataManager.h"
#include "render/Renderer.h"
#include "render/Camera.h"
#include "ui/UIManager.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <memory>
#include <thread>
#include <atomic>

namespace terrafirma {

class Application {
public:
    Application();
    ~Application();

    bool init();
    void run();
    void shutdown();

    // Callbacks
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);

private:
    void processInput();
    void update(float deltaTime);
    void render();
    void networkThreadFunc();

    GLFWwindow* m_window = nullptr;
    int m_windowWidth = 1280;
    int m_windowHeight = 720;
    bool m_fullscreen = false;
    int m_windowedX = 100, m_windowedY = 100;
    int m_windowedWidth = 1280, m_windowedHeight = 720;

    Timer m_timer;
    std::unique_ptr<DataManager> m_dataManager;
    std::unique_ptr<UDPReceiver> m_networkReceiver;
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<Camera> m_camera;
    std::unique_ptr<UIManager> m_uiManager;

    // Network thread
    std::thread m_networkThread;
    std::atomic<bool> m_running{false};

    // Input state
    bool m_firstMouse = true;
    float m_lastX = 640.0f;
    float m_lastY = 360.0f;
    bool m_mouseCapture = false;

    // Selected rover
    int m_selectedRover = 0;
    bool m_followRover = false;

    // Render settings
    RenderSettings m_renderSettings;
};

} // namespace terrafirma

