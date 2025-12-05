#include "render/Camera.h"
#include <algorithm>
#include <cmath>

namespace terrafirma {

// COORDINATE SYSTEM: X=horizontal, Y=UP (height), Z=horizontal (forward)

Camera::Camera(glm::vec3 position)
    : m_position(position)
    , m_worldUp(0.0f, 1.0f, 0.0f) // Y is up!
    , m_yaw(-90.0f)   // Looking along +Z initially
    , m_pitch(-30.0f) // Looking slightly downward
{
    updateVectors();
}

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(m_position, m_position + m_front, m_up);
}

void Camera::lookAt(const glm::vec3& target) {
    glm::vec3 direction = target - m_position;
    float dist = glm::length(direction);
    if (dist < 0.001f) return;
    
    direction = direction / dist;
    
    // Calculate pitch (angle from XZ plane, positive = up)
    m_pitch = glm::degrees(asin(glm::clamp(direction.y, -1.0f, 1.0f)));
    
    // Calculate yaw (angle in XZ plane from +Z axis)
    m_yaw = glm::degrees(atan2(direction.x, direction.z));
    
    updateVectors();
}

// W/S: Move along camera's forward direction (full 3D)
void Camera::moveForward(float delta) {
    m_position += m_front * delta;
}

// A/D: Strafe along camera's right direction (full 3D)
void Camera::moveRight(float delta) {
    m_position += m_right * delta;
}

// Space/Shift: Move along world up (Y axis)
void Camera::moveUp(float delta) {
    m_position.y += delta;
}

void Camera::rotate(float yawDelta, float pitchDelta) {
    m_yaw += yawDelta;
    m_pitch += pitchDelta;
    m_pitch = std::clamp(m_pitch, -89.0f, 89.0f);
    updateVectors();
}

void Camera::zoom(float delta) {
    m_position += m_front * delta;
}

void Camera::updateVectors() {
    // Calculate front vector from yaw and pitch
    // Yaw: angle in XZ plane from +Z axis
    // Pitch: angle from XZ plane (positive = up)
    float yawRad = glm::radians(m_yaw);
    float pitchRad = glm::radians(m_pitch);
    
    m_front.x = sin(yawRad) * cos(pitchRad);
    m_front.y = sin(pitchRad);
    m_front.z = cos(yawRad) * cos(pitchRad);
    m_front = glm::normalize(m_front);
    
    // Right vector is perpendicular to front and world up
    m_right = glm::normalize(glm::cross(m_front, m_worldUp));
    
    // Camera up vector
    m_up = glm::normalize(glm::cross(m_right, m_front));
}

} // namespace terrafirma
