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

private:
    RoverState m_state;
    
    // Target values for interpolation
    glm::vec3 m_targetPosition{0.0f};
    glm::vec3 m_targetRotation{0.0f};
    bool m_hasTarget = false;
};

} // namespace terrafirma

