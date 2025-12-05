#pragma once

#include "common.h"

namespace terrafirma {

class RoverData {
public:
    RoverData(int id);

    void updatePose(const PosePacket& pose);
    void updateTelemetry(const VehicleTelem& telem);
    void interpolate(float deltaTime); // Smooth position/rotation
    
    const RoverState& getState() const { return m_state; }
    RoverState& getState() { return m_state; }
    
    // Engine control (button 0)
    bool isEngineRunning() const { return m_engineRunning; }
    void setEngineRunning(bool running);

private:
    RoverState m_state;
    
    // Target values for interpolation
    glm::vec3 m_targetPosition{0.0f};
    glm::vec3 m_targetRotation{0.0f};
    bool m_hasTarget = false;
    
    // Engine state (controlled by button 0)
    bool m_engineRunning = true;
    
    // Resume smoothing - when resuming from pause, use slower interpolation
    bool m_justResumed = false;
    float m_resumeTimer = 0.0f;
    static constexpr float RESUME_SMOOTH_DURATION = 2.0f; // seconds to smoothly blend after resume
};

} // namespace terrafirma
