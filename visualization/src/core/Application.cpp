#include "core/Application.h"
#include <iostream>
#include <algorithm>
#include <chrono>

namespace terrafirma {

static Application* g_app = nullptr;

Application::Application() {
    g_app = this;
}

Application::~Application() {
    shutdown();
    g_app = nullptr;
}

bool Application::init() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return false;
    }

    // OpenGL 3.3 Core Profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    // Create window
    m_window = glfwCreateWindow(m_windowWidth, m_windowHeight, "Terrafirma Rovers - Command Center", nullptr, nullptr);
    if (!m_window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1); // VSync

    // Set callbacks
    glfwSetKeyCallback(m_window, keyCallback);
    glfwSetMouseButtonCallback(m_window, mouseButtonCallback);
    glfwSetCursorPosCallback(m_window, cursorPosCallback);
    glfwSetScrollCallback(m_window, scrollCallback);
    glfwSetFramebufferSizeCallback(m_window, framebufferSizeCallback);

    // Load OpenGL functions
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return false;
    }

    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << "\n";

    // Initialize components
    m_dataManager = std::make_unique<DataManager>();
    m_networkReceiver = std::make_unique<UDPReceiver>(m_dataManager.get());
    // Coordinate system: X=horizontal, Y=height (UP), Z=horizontal (forward)
    // Data: X=50-500, Y=30-80 (height), Z=100-400
    // Position camera above (high Y) and behind (low Z) looking forward (+Z)
    m_camera = std::make_unique<Camera>(glm::vec3(300.0f, 150.0f, 0.0f));
    m_renderer = std::make_unique<Renderer>();
    m_uiManager = std::make_unique<UIManager>(m_window);

    if (!m_renderer->init()) {
        std::cerr << "Failed to initialize renderer\n";
        return false;
    }

    if (!m_networkReceiver->init()) {
        std::cerr << "Failed to initialize network receiver\n";
        return false;
    }

    // Start network thread
    m_running = true;
    m_networkThread = std::thread(&Application::networkThreadFunc, this);

    std::cout << "Application initialized successfully\n";
    return true;
}

void Application::run() {
    while (!glfwWindowShouldClose(m_window) && m_running) {
        m_timer.update();
        float deltaTime = m_timer.getDeltaTime();

        processInput();
        update(deltaTime);
        render();

        glfwSwapBuffers(m_window);
        glfwPollEvents();
    }
}

void Application::shutdown() {
    m_running = false;
    
    if (m_networkThread.joinable()) {
        m_networkThread.join();
    }

    m_uiManager.reset();
    m_renderer.reset();
    m_networkReceiver.reset();
    m_dataManager.reset();

    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
    glfwTerminate();
}

void Application::processInput() {
    // Camera movement with WASD - camera-relative (forward = where camera looks in XY plane)
    float speed = m_camera->getSpeed() * m_timer.getDeltaTime();
    
    // W/S - move forward/back relative to camera direction
    if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS)
        m_camera->moveForward(speed);
    if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS)
        m_camera->moveForward(-speed);
    
    // A/D - strafe left/right relative to camera direction
    if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS)
        m_camera->moveRight(-speed);
    if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS)
        m_camera->moveRight(speed);
    
    // Space - move up (always along Y axis)
    if (glfwGetKey(m_window, GLFW_KEY_SPACE) == GLFW_PRESS)
        m_camera->moveUp(speed);
}

void Application::update(float deltaTime) {
    // Update data manager (interpolation)
    m_dataManager->update(deltaTime);
    
    // Update rover online status using same time source as RoverData
    static auto startTime = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    double currentTime = std::chrono::duration<double>(now - startTime).count();
    
    for (int i = 0; i < NUM_ROVERS; i++) {
        auto& rover = m_dataManager->getRover(i);
        rover.online = (currentTime - rover.lastTimestamp) < OFFLINE_TIMEOUT;
    }

    // Follow selected rover if enabled
    if (m_followRover) {
        auto& rover = m_dataManager->getRover(m_selectedRover);
        // Position camera behind and above rover (in XY plane, Z is up)
        glm::vec3 targetPos = rover.position + glm::vec3(-50, 0, 50);
        m_camera->setPosition(glm::mix(m_camera->getPosition(), targetPos, deltaTime * 3.0f));
        m_camera->lookAt(rover.position);
    }
}

void Application::render() {
    glClearColor(0.0f, 0.0f, 0.04f, 1.0f); // Deep space black
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Get window size
    int width, height;
    glfwGetFramebufferSize(m_window, &width, &height);
    
    glm::mat4 view = m_camera->getViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 
        (float)width / (float)height, 0.1f, 10000.0f);

    // Render 3D scene
    m_renderer->begin(view, projection);
    
    // Render rovers
    for (int i = 0; i < NUM_ROVERS; i++) {
        auto& rover = m_dataManager->getRover(i);
        bool selected = (i == m_selectedRover);
        bool engineRunning = m_dataManager->isRoverEngineRunning(i);
        m_renderer->renderRover(rover, ROVER_COLORS[i], selected, engineRunning);
    }

    // Render point clouds (each rover has its own GPU buffer)
    if (m_renderSettings.showPointCloud) {
        for (int i = 0; i < NUM_ROVERS; i++) {
            auto& pointCloud = m_dataManager->getPointCloud(i);
            m_renderer->renderPointCloud(i, pointCloud, m_renderSettings);
        }
    }

    // Render terrain
    if (m_renderSettings.showTerrain) {
        m_renderer->renderTerrain(m_dataManager->getTerrainGrid(), m_renderSettings);
    }

    m_renderer->end();

    // Render UI
    m_uiManager->begin();
    m_uiManager->renderMainUI(
        m_dataManager.get(),
        m_networkReceiver.get(),
        m_selectedRover,
        m_followRover,
        m_renderSettings,
        m_timer.getFPS(),
        m_camera.get()
    );
    m_uiManager->end();
}

void Application::networkThreadFunc() {
    while (m_running) {
        m_networkReceiver->update();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

// Callbacks
void Application::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    (void)scancode;
    (void)mods;
    
    if (!g_app) return;

    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                if (g_app->m_mouseCapture) {
                    g_app->m_mouseCapture = false;
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                } else {
                    glfwSetWindowShouldClose(window, GLFW_TRUE);
                }
                break;
            case GLFW_KEY_F11:
                // Toggle fullscreen
                if (!g_app->m_fullscreen) {
                    glfwGetWindowPos(window, &g_app->m_windowedX, &g_app->m_windowedY);
                    glfwGetWindowSize(window, &g_app->m_windowedWidth, &g_app->m_windowedHeight);
                    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
                    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
                    glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
                    g_app->m_fullscreen = true;
                } else {
                    glfwSetWindowMonitor(window, nullptr, 
                        g_app->m_windowedX, g_app->m_windowedY,
                        g_app->m_windowedWidth, g_app->m_windowedHeight, 0);
                    g_app->m_fullscreen = false;
                }
                break;
            case GLFW_KEY_F:
                g_app->m_followRover = !g_app->m_followRover;
                break;
            case GLFW_KEY_1:
            case GLFW_KEY_2:
            case GLFW_KEY_3:
            case GLFW_KEY_4:
            case GLFW_KEY_5:
                g_app->m_selectedRover = key - GLFW_KEY_1;
                break;
        }
    }
}

void Application::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    (void)mods;
    
    if (!g_app) return;
    
    // Don't capture mouse if ImGui wants it
    if (g_app->m_uiManager && g_app->m_uiManager->wantCaptureMouse()) {
        return;
    }

    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        g_app->m_mouseCapture = true;
        // Skip the next few cursor events to avoid snapping from GLFW cursor warp
        g_app->m_skipCursorEvents = 2;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
        g_app->m_mouseCapture = false;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

void Application::cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    (void)window;
    
    if (!g_app || !g_app->m_mouseCapture) return;

    float x = static_cast<float>(xpos);
    float y = static_cast<float>(ypos);

    // Skip initial cursor events after enabling capture to prevent snapping
    if (g_app->m_skipCursorEvents > 0) {
        g_app->m_skipCursorEvents--;
        g_app->m_lastX = x;
        g_app->m_lastY = y;
        return;  // Don't rotate, just record position
    }

    float xoffset = x - g_app->m_lastX;
    float yoffset = g_app->m_lastY - y; // Reversed

    g_app->m_lastX = x;
    g_app->m_lastY = y;

    // Invert horizontal mouse control
    g_app->m_camera->rotate(-xoffset * 0.1f, yoffset * 0.1f);
}

void Application::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    (void)window;
    (void)xoffset;
    
    if (!g_app) return;
    
    // Don't process scroll if ImGui wants it
    if (g_app->m_uiManager && g_app->m_uiManager->wantCaptureMouse()) {
        return;
    }

    g_app->m_camera->zoom(static_cast<float>(yoffset) * 5.0f);
}

void Application::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    (void)window;
    
    if (width > 0 && height > 0) {
        glViewport(0, 0, width, height);
        if (g_app) {
            g_app->m_windowWidth = width;
            g_app->m_windowHeight = height;
        }
    }
}

} // namespace terrafirma

