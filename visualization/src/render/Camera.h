#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace terrafirma {

class Camera {
public:
    Camera(glm::vec3 position = glm::vec3(400.0f, -100.0f, 300.0f));

    glm::mat4 getViewMatrix() const;
    glm::vec3 getPosition() const { return m_position; }
    
    void setPosition(const glm::vec3& pos) { m_position = pos; }
    void lookAt(const glm::vec3& target);
    
    // Camera-relative movement (W = forward along camera direction in XY plane)
    void moveForward(float delta);  // W/S
    void moveRight(float delta);    // A/D
    void moveUp(float delta);       // Space/Shift
    
    void rotate(float yawDelta, float pitchDelta);
    void zoom(float delta);
    
    float getSpeed() const { return m_speed; }
    void setSpeed(float speed) { m_speed = speed; }
    
    float getYaw() const { return m_yaw; }
    float getPitch() const { return m_pitch; }

private:
    void updateVectors();

    glm::vec3 m_position;
    glm::vec3 m_front;
    glm::vec3 m_up;
    glm::vec3 m_right;
    glm::vec3 m_worldUp;

    float m_yaw = 90.0f;   // Looking along +Y
    float m_pitch = -45.0f;
    float m_speed = 100.0f;
};

} // namespace terrafirma
