#pragma once

#include <chrono>

namespace terrafirma {

class Timer {
public:
    Timer();
    void update();
    float getDeltaTime() const { return m_deltaTime; }
    float getFPS() const { return m_fps; }
    double getTime() const;

private:
    std::chrono::high_resolution_clock::time_point m_lastTime;
    float m_deltaTime = 0.0f;
    float m_fps = 0.0f;
    float m_fpsAccum = 0.0f;
    int m_frameCount = 0;
};

} // namespace terrafirma

