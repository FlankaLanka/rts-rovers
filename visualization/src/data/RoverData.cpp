#include "data/RoverData.h"
#include "TimeUtil.h"
#include <algorithm>
#include <cmath>

namespace terrafirma {

// Use shared time utility for consistent timestamps
static double getCurrentTime() {
    return TimeUtil::getTime();
}

RoverData::RoverData(int id) {
    m_state.id = id;
}

void RoverData::updatePose(const PosePacket& pose) {
    m_targetPosition = glm::vec3(pose.posX, pose.posY, pose.posZ);
    m_targetRotation = glm::vec3(pose.rotXdeg, pose.rotYdeg, pose.rotZdeg);
    
    if (!m_hasTarget) {
        // First update - snap to position immediately
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
    
    // Sync engine state with button 0 from telemetry
    bool newEngineState = (telem.buttonStates & 1) != 0;
    if (newEngineState && !m_engineRunning) {
        // Engine is being turned ON - trigger smooth resume
        m_justResumed = true;
        m_resumeTimer = 0.0f;
    }
    m_engineRunning = newEngineState;
}

void RoverData::setEngineRunning(bool running) {
    if (running && !m_engineRunning) {
        // Resuming from pause - enable smooth transition
        m_justResumed = true;
        m_resumeTimer = 0.0f;
    }
    m_engineRunning = running;
}

void RoverData::interpolate(float deltaTime) {
    if (!m_hasTarget) return;
    
    // Update resume timer
    if (m_justResumed) {
        m_resumeTimer += deltaTime;
        if (m_resumeTimer >= RESUME_SMOOTH_DURATION) {
            m_justResumed = false;
        }
    }
    
    // EXPONENTIAL SMOOTHING - frame-rate independent!
    // 
    // This formula gives the same visual result whether you're
    // running at 30fps, 60fps, or 144fps.
    //
    // The "10.0f" controls how fast we catch up:
    // - Higher = snappier (reaches target faster)
    // - Lower = smoother (takes longer to reach target)
    //
    // At 10.0f, we reach ~95% of target in 0.3 seconds
    
    float baseSpeed = 10.0f;
    
    // If we just resumed from pause, use MUCH slower interpolation
    // to smoothly blend from paused position to current data position
    if (m_justResumed) {
        // Start very slow (1.0) and gradually increase to normal (10.0)
        float resumeProgress = m_resumeTimer / RESUME_SMOOTH_DURATION;
        float smoothProgress = resumeProgress * resumeProgress; // Ease-in curve
        baseSpeed = glm::mix(1.0f, 10.0f, smoothProgress);
    }
    
    float smoothingFactor = 1.0f - std::exp(-deltaTime * baseSpeed);
    
    // Smoothly move position toward target
    m_state.position = glm::mix(m_state.position, m_targetPosition, smoothingFactor);
    
    // Smoothly rotate toward target (with angle wrapping)
    for (int i = 0; i < 3; i++) {
        float diff = m_targetRotation[i] - m_state.rotation[i];
        
        // Handle angle wrap-around (e.g., 359° to 1° should go +2°, not -358°)
        while (diff > 180.0f) diff -= 360.0f;
        while (diff < -180.0f) diff += 360.0f;
        
        m_state.rotation[i] += diff * smoothingFactor;
        
        // Normalize angle to [-180, 180] range to prevent drift
        while (m_state.rotation[i] > 180.0f) m_state.rotation[i] -= 360.0f;
        while (m_state.rotation[i] < -180.0f) m_state.rotation[i] += 360.0f;
    }
}

} // namespace terrafirma
