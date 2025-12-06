#pragma once

#include "common.h"
#include "core/Timer.h"
#include "network/UDPReceiver.h"
#include "data/DataManager.h"
#include "render/Renderer.h"
#include "render/Camera.h"
#include "render/CircleRenderer.h"
#include "terrain/TerrainOperation.h"
#include "pathfinding/AStar.h"
#include "pathfinding/PathRenderer.h"
#include "ui/UIManager.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <memory>
#include <thread>
#include <atomic>
#include <vector>

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
    void handleCircleDrawing(double mouseX, double mouseY, bool pressed, bool released);
    void handleRTSClick(double mouseX, double mouseY);
    void updatePathMovement(int roverIndex, float deltaTime);
    void spawnWaypoint(int roverIndex);

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
    std::unique_ptr<CircleRenderer> m_circleRenderer;
    std::unique_ptr<TerrainOperationManager> m_opManager;

    // Network thread
    std::thread m_networkThread;
    std::atomic<bool> m_running{false};

    // Input state
    float m_lastX = 640.0f;
    float m_lastY = 360.0f;
    bool m_mouseCapture = false;
    int m_skipCursorEvents = 0;  // Skip N cursor events after enabling capture

    // Selected rover
    int m_selectedRover = 0;
    bool m_followRover = false;
    
    // Manual control mode (Button 3) - per rover
    std::array<bool, NUM_ROVERS> m_manualControl{};
    
    // RTS mode (Button 4) - click to pathfind
    std::array<bool, NUM_ROVERS> m_rtsMode{};
    
    // WAY mode (Button 5) - auto waypoint
    std::array<bool, NUM_ROVERS> m_wayMode{};
    
    // Pathfinding state per rover
    std::array<std::vector<glm::vec3>, NUM_ROVERS> m_currentPath{};
    std::array<size_t, NUM_ROVERS> m_pathIndex{};       // Current waypoint index
    std::array<glm::vec3, NUM_ROVERS> m_pathDestination{};  // Final destination
    std::array<bool, NUM_ROVERS> m_hasPath{};           // Whether rover has active path
    
    // Pathfinding and rendering
    std::unique_ptr<AStar> m_pathfinder;
    std::unique_ptr<PathRenderer> m_pathRenderer;

    // Render settings
    RenderSettings m_renderSettings;
    
    // Circle drawing state
    bool m_isDrawingCircle = false;
    glm::vec2 m_circleStart{0.0f};
    glm::vec2 m_circleCenter{0.0f};
    float m_circleRadius = 0.0f;
    
    // 3rd person camera settings
    static constexpr float THIRD_PERSON_DISTANCE = 30.0f;
    static constexpr float THIRD_PERSON_HEIGHT = 15.0f;
    static constexpr float ROVER_MOVE_SPEED = 20.0f;
    
    // Pathfinding movement settings
    static constexpr float PATH_MOVE_SPEED = 10.0f;      // Meters per second
    static constexpr float PATH_WAYPOINT_DIST = 2.0f;    // Distance to consider waypoint reached
    static constexpr float PATH_HOVER_HEIGHT = 3.0f;     // Hover height above terrain
};

} // namespace terrafirma
