#include "core/Application.h"
#include "terrain/TerrainRaycast.h"
#include "TimeUtil.h"
#include <iostream>
#include <algorithm>
#include <chrono>
#include <cmath>

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
    m_circleRenderer = std::make_unique<CircleRenderer>();
    m_opManager = std::make_unique<TerrainOperationManager>();

    if (!m_renderer->init()) {
        std::cerr << "Failed to initialize renderer\n";
        return false;
    }

    if (!m_circleRenderer->init()) {
        std::cerr << "Failed to initialize circle renderer\n";
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
    m_circleRenderer.reset();
    m_opManager.reset();
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
    float deltaTime = m_timer.getDeltaTime();
    
    // Check if we're in manual control mode for selected rover
    bool inManualControl = m_manualControl[m_selectedRover];
    
    if (inManualControl) {
        // Manual control: WASD moves the ROVER like a vehicle
        auto& rover = m_dataManager->getRover(m_selectedRover);
        float roverSpeed = ROVER_MOVE_SPEED * deltaTime;
        
        // Construction vehicles only rotate around Y axis (yaw only)
        // In Y-up system: rotation.y = yaw (around Y axis)
        // Clear roll and pitch completely
        rover.rotation.x = 0.0f;  // No roll
        rover.rotation.z = 0.0f;  // No pitch (in Y-up, Z is forward/back tilt)
        
        // Get rover's forward direction from yaw (rotation.y = yaw around Y axis)
        float yawRad = glm::radians(rover.rotation.y);
        // In Y-up: forward is along XZ plane, Y is up
        // When yaw=0, forward is +Z. When yaw=90, forward is +X
        glm::vec3 forward(-std::sin(yawRad), 0.0f, std::cos(yawRad));
        
        // W/S - move rover forward/back
        if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS) {
            rover.position += forward * roverSpeed;
        }
        if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS) {
            rover.position -= forward * roverSpeed;
        }
        
        // A/D - turn rover left/right (rotate around Y axis like a real vehicle)
        float turnSpeed = 60.0f * deltaTime;  // 60 degrees per second
        if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS) {
            rover.rotation.y -= turnSpeed;  // Turn left
        }
        if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS) {
            rover.rotation.y += turnSpeed;  // Turn right
        }
        
        // Keep yaw in [-180, 180] range
        while (rover.rotation.y > 180.0f) rover.rotation.y -= 360.0f;
        while (rover.rotation.y < -180.0f) rover.rotation.y += 360.0f;
        
        // Maintain rover height above terrain
        float terrainHeight = m_dataManager->getTerrainGrid().getMinHeight();
        getTerrainHeightAt(m_dataManager->getTerrainGrid(), rover.position.x, rover.position.z, terrainHeight);
        rover.position.y = terrainHeight + 2.0f;  // Hover slightly above terrain
    } else {
        // Normal mode: WASD moves the CAMERA
        float speed = m_camera->getSpeed() * deltaTime;
        
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
}

void Application::update(float deltaTime) {
    // Set manual control flags for all rovers (blocks UDP updates)
    for (int i = 0; i < NUM_ROVERS; i++) {
        // Rover is controlled if in manual mode OR in a dig/pile operation
        bool controlled = m_manualControl[i] || m_opManager->isRoverControlled(i);
        m_dataManager->setRoverControlled(i, controlled);
    }
    
    // Update data manager (interpolation - skipped for controlled rovers)
    m_dataManager->update(deltaTime);
    
    // Update terrain operations
    if (m_opManager) {
        m_opManager->update(deltaTime, *m_dataManager);
    }
    
    // Update rover online status using shared time source
    double currentTime = TimeUtil::getTime();
    
    for (int i = 0; i < NUM_ROVERS; i++) {
        auto& rover = m_dataManager->getRover(i);
        rover.online = (currentTime - rover.lastTimestamp) < OFFLINE_TIMEOUT;
    }

    // 3rd person camera for manual control OR follow mode
    bool inManualControl = m_manualControl[m_selectedRover];
    if (inManualControl || m_followRover) {
        auto& rover = m_dataManager->getRover(m_selectedRover);
        
        // Calculate camera position behind the rover based on its yaw rotation
        // In Y-up system: rotation.y = yaw (rotation around Y axis)
        float yawRad = glm::radians(rover.rotation.y);
        
        // Forward direction of rover (where it's facing)
        // When yaw=0, forward is +Z. When yaw=90, forward is -X
        glm::vec3 forward(-std::sin(yawRad), 0.0f, std::cos(yawRad));
        
        // Camera is BEHIND the rover (opposite of forward direction)
        glm::vec3 behindOffset = -forward * THIRD_PERSON_DISTANCE;
        behindOffset.y = THIRD_PERSON_HEIGHT;  // Above the rover
        
        glm::vec3 targetCamPos = rover.position + behindOffset;
        
        // Smoothly move camera to target position
        float smoothFactor = inManualControl ? 10.0f : 3.0f;  // Faster follow for manual control
        m_camera->setPosition(glm::mix(m_camera->getPosition(), targetCamPos, deltaTime * smoothFactor));
        
        // Look at a point IN FRONT of the rover (where it's heading)
        // This makes the camera face the same direction as the rover
        glm::vec3 lookTarget = rover.position + forward * 20.0f + glm::vec3(0, 3.0f, 0);
        m_camera->lookAt(lookTarget);
    }
    
    // Sync circle drawing state with UI
    if (m_uiManager) {
        auto& op = m_opManager->getOperation(m_selectedRover);
        m_isDrawingCircle = (op.getState() == OperationState::DRAWING);
    }
}

void Application::handleCircleDrawing(double mouseX, double mouseY, bool pressed, bool released) {
    if (!m_opManager) return;
    
    auto& op = m_opManager->getOperation(m_selectedRover);
    
    if (op.getState() != OperationState::DRAWING) {
        m_isDrawingCircle = false;
        return;
    }
    
    // Get both window size and framebuffer size (may differ on Retina displays)
    int fbWidth, fbHeight;
    int winWidth, winHeight;
    glfwGetFramebufferSize(m_window, &fbWidth, &fbHeight);
    glfwGetWindowSize(m_window, &winWidth, &winHeight);
    
    // Convert mouse coordinates from window space to framebuffer space
    float scaleX = (float)fbWidth / (float)winWidth;
    float scaleY = (float)fbHeight / (float)winHeight;
    float fbMouseX = (float)mouseX * scaleX;
    float fbMouseY = (float)mouseY * scaleY;
    
    glm::mat4 view = m_camera->getViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 
        (float)fbWidth / (float)fbHeight, 0.1f, 10000.0f);
    
    // Raycast to terrain using framebuffer coordinates
    RaycastResult hit = raycastTerrain(
        fbMouseX, fbMouseY,
        fbWidth, fbHeight, view, projection,
        m_dataManager->getTerrainGrid()
    );
    
    // Fallback: if no terrain hit, intersect with a horizontal plane at average terrain height
    if (!hit.hit) {
        auto& terrain = m_dataManager->getTerrainGrid();
        float planeY = (terrain.getMinHeight() + terrain.getMaxHeight()) * 0.5f;
        if (terrain.getCells().empty()) {
            planeY = 50.0f;  // Default height if no terrain data
        }
        
        // Intersect ray with horizontal plane at planeY
        glm::vec3 rayOrigin, rayDir;
        screenToWorldRay(fbMouseX, fbMouseY,
                        fbWidth, fbHeight, view, projection, rayOrigin, rayDir);
        
        // Plane intersection: solve rayOrigin.y + t * rayDir.y = planeY
        if (std::abs(rayDir.y) > 0.0001f) {
            float t = (planeY - rayOrigin.y) / rayDir.y;
            if (t > 0.0f) {
                hit.hit = true;
                hit.position = rayOrigin + rayDir * t;
                hit.distance = t;
            }
        }
    }
    
    if (pressed && hit.hit) {
        // Start drawing - record center
        m_circleStart = glm::vec2(hit.position.x, hit.position.z);
        m_circleCenter = m_circleStart;
        m_circleRadius = 0.0f;
        m_isDrawingCircle = true;  // Mark that we've started drawing
        op.updateDrawing(m_circleCenter, 0.0f);
        std::cout << "Circle drawing started at (" << m_circleStart.x << ", " << m_circleStart.y << ")\n";
    } else if (m_isDrawingCircle && hit.hit) {
        // Update radius during drag
        glm::vec2 currentPos(hit.position.x, hit.position.z);
        m_circleRadius = glm::length(currentPos - m_circleStart);
        m_circleCenter = m_circleStart;
        op.updateDrawing(m_circleCenter, m_circleRadius);
    }
    
    if (released && m_circleRadius > 0.5f) {
        // Finish drawing
        std::cout << "Circle drawing finished: center=(" << m_circleCenter.x << ", " << m_circleCenter.y 
                  << ") radius=" << m_circleRadius << "\n";
        op.finishDrawing();
        m_isDrawingCircle = false;
    } else if (released) {
        // Released but circle too small - stay in drawing state
        m_isDrawingCircle = false;
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
    
    // Render operation circles
    if (m_opManager && m_circleRenderer) {
        for (int i = 0; i < NUM_ROVERS; i++) {
            auto& op = m_opManager->getOperation(i);
            if (op.isActive() && op.getRadius() > 0.1f) {
                // Red semi-transparent circle for dig/pile area
                glm::vec4 circleColor(1.0f, 0.2f, 0.2f, 0.5f);  // Red with 50% alpha
                
                // Different tint based on operation type
                if (op.getType() == OperationType::PILE) {
                    circleColor = glm::vec4(0.8f, 0.5f, 0.2f, 0.5f);  // Orange for pile
                }
                
                m_circleRenderer->render(
                    op.getCenter(), op.getRadius(), circleColor,
                    m_dataManager->getTerrainGrid(),
                    view, projection
                );
            }
        }
    }

    // Render UI
    m_uiManager->begin();
    m_uiManager->renderMainUI(
        m_dataManager.get(),
        m_networkReceiver.get(),
        m_selectedRover,
        m_followRover,
        m_renderSettings,
        m_timer.getFPS(),
        m_camera.get(),
        m_opManager.get(),
        &m_manualControl
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
                // Cancel any active circle drawing first
                if (g_app->m_opManager) {
                    auto& op = g_app->m_opManager->getOperation(g_app->m_selectedRover);
                    if (op.getState() == OperationState::DRAWING || 
                        op.getState() == OperationState::CONFIRMING) {
                        op.cancel();
                        g_app->m_isDrawingCircle = false;
                        return;
                    }
                }
                
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
    
    // Handle circle drawing with left mouse button
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);
        
        bool pressed = (action == GLFW_PRESS);
        bool released = (action == GLFW_RELEASE);
        
        // Check if we should be drawing
        if (g_app->m_opManager) {
            auto& op = g_app->m_opManager->getOperation(g_app->m_selectedRover);
            if (op.getState() == OperationState::DRAWING) {
                g_app->handleCircleDrawing(mouseX, mouseY, pressed, released);
                return;
            }
        }
    }

    // Camera rotation with right mouse button
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
    
    if (!g_app) return;
    
    // Handle circle drawing updates during drag
    if (g_app->m_opManager) {
        auto& op = g_app->m_opManager->getOperation(g_app->m_selectedRover);
        if (op.getState() == OperationState::DRAWING && 
            glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            g_app->handleCircleDrawing(xpos, ypos, false, false);
        }
    }
    
    if (!g_app->m_mouseCapture) return;
    
    // Disable camera rotation in manual control mode
    if (g_app->m_manualControl[g_app->m_selectedRover]) {
        return;
    }

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
