#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <array>
#include <mutex>
#include <atomic>
#include <chrono>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace terrafirma {

// Number of rovers
constexpr int NUM_ROVERS = 5;

// Network ports
constexpr int POSE_PORT_BASE = 9000;
constexpr int LIDAR_PORT_BASE = 10000;
constexpr int TELEM_PORT_BASE = 11000;
constexpr int CMD_PORT_BASE = 8000;

// LiDAR constants
constexpr size_t MAX_LIDAR_POINTS_PER_PACKET = 100;

// Packet structures (must match emulator)
#pragma pack(push, 1)
struct PosePacket {
    double timestamp;
    float posX;
    float posY;
    float posZ;
    float rotXdeg;
    float rotYdeg;
    float rotZdeg;
};

struct LidarPacketHeader {
    double timestamp;
    uint32_t chunkIndex;
    uint32_t totalChunks;
    uint32_t pointsInThisChunk;
};

struct LidarPoint {
    float x;
    float y;
    float z;
};

struct LidarPacket {
    LidarPacketHeader header;
    LidarPoint points[MAX_LIDAR_POINTS_PER_PACKET];
};

struct VehicleTelem {
    double timestamp;
    uint8_t buttonStates;
};
#pragma pack(pop)

// Rover state
struct RoverState {
    int id = 0;
    glm::vec3 position{0.0f};
    glm::vec3 rotation{0.0f}; // roll, pitch, yaw in degrees
    uint8_t buttonStates = 0;
    double lastTimestamp = 0.0;
    bool online = false;
    
    // Derived - Y-up coordinate system
    // rotation.x = roll (around X axis - unused for construction vehicles)
    // rotation.y = yaw (around Y axis - turning left/right)
    // rotation.z = pitch (around Z axis - unused for construction vehicles)
    glm::mat4 getModelMatrix() const {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, position);
        // Apply rotations: yaw first (Y axis), then pitch (Z), then roll (X)
        // Negate yaw to match camera rotation direction
        model = glm::rotate(model, glm::radians(-rotation.y), glm::vec3(0, 1, 0)); // yaw around Y
        model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0, 0, 1)); // pitch around Z
        model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1, 0, 0)); // roll around X
        return model;
    }
};

// Rover colors (unique per rover)
const glm::vec3 ROVER_COLORS[NUM_ROVERS] = {
    glm::vec3(0.0f, 1.0f, 1.0f),   // Cyan
    glm::vec3(1.0f, 0.0f, 1.0f),   // Magenta
    glm::vec3(1.0f, 1.0f, 0.0f),   // Yellow
    glm::vec3(0.0f, 1.0f, 0.53f),  // Green
    glm::vec3(1.0f, 0.53f, 0.0f),  // Orange
};

// Height gradient colors
inline glm::vec3 heightToColor(float normalizedHeight) {
    // Blue -> Cyan -> Green -> Yellow -> Red
    if (normalizedHeight < 0.25f) {
        float t = normalizedHeight / 0.25f;
        return glm::mix(glm::vec3(0.0f, 0.4f, 1.0f), glm::vec3(0.0f, 1.0f, 1.0f), t);
    } else if (normalizedHeight < 0.5f) {
        float t = (normalizedHeight - 0.25f) / 0.25f;
        return glm::mix(glm::vec3(0.0f, 1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.4f), t);
    } else if (normalizedHeight < 0.75f) {
        float t = (normalizedHeight - 0.5f) / 0.25f;
        return glm::mix(glm::vec3(0.0f, 1.0f, 0.4f), glm::vec3(1.0f, 1.0f, 0.0f), t);
    } else {
        float t = (normalizedHeight - 0.75f) / 0.25f;
        return glm::mix(glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(1.0f, 0.27f, 0.0f), t);
    }
}

// Render settings
struct RenderSettings {
    bool showTerrain = true;
    bool terrainSolid = true;
    bool terrainWireframe = false;
    bool terrainHeightColors = true;
    bool showPointCloud = true;
    bool pointCloudHeightColors = true;
    float pointSize = 2.0f;
};

// Offline timeout (seconds)
constexpr double OFFLINE_TIMEOUT = 2.0;

} // namespace terrafirma

