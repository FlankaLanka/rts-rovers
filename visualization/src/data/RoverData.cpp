#include "data/RoverData.h"
#include <algorithm>
#include <chrono>

namespace terrafirma {

// Thread-safe time function (doesn't use GLFW)
static double getCurrentTime() {
    static auto startTime = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration<double>(now - startTime).count();
}

RoverData::RoverData(int id) {
    m_state.id = id;
}

void RoverData::updatePose(const PosePacket& pose) {
    m_targetPosition = glm::vec3(pose.posX, pose.posY, pose.posZ);
    m_targetRotation = glm::vec3(pose.rotXdeg, pose.rotYdeg, pose.rotZdeg);
    
    if (!m_hasTarget) {
        // First update - snap to position
        m_state.position = m_targetPosition;
        m_state.rotation = m_targetRotation;
        m_hasTarget = true;
    }
    
    m_state.lastTimestamp = getCurrentTime();
    m_state.online = true;
}

void RoverData::updateTelemetry(const VehicleTelem& telem) {
    m_state.buttonStates = telem.buttonStates;
    m_state.lastTimestamp = getCurrentTime();
    m_state.online = true;
}

void RoverData::interpolate(float deltaTime) {
    if (!m_hasTarget) return;
    
    // Smooth interpolation towards target
    float lerpFactor = std::min(1.0f, deltaTime * 15.0f); // Smooth factor
    
    m_state.position = glm::mix(m_state.position, m_targetPosition, lerpFactor);
    
    // Interpolate rotation (handle angle wrapping)
    for (int i = 0; i < 3; i++) {
        float diff = m_targetRotation[i] - m_state.rotation[i];
        // Handle wrap-around for angles
        while (diff > 180.0f) diff -= 360.0f;
        while (diff < -180.0f) diff += 360.0f;
        m_state.rotation[i] += diff * lerpFactor;
    }
}

} // namespace terrafirma

