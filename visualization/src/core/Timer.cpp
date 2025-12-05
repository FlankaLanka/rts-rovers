#include "core/Timer.h"

namespace terrafirma {

Timer::Timer() 
    : m_lastTime(std::chrono::high_resolution_clock::now()) {
}

void Timer::update() {
    auto currentTime = std::chrono::high_resolution_clock::now();
    m_deltaTime = std::chrono::duration<float>(currentTime - m_lastTime).count();
    m_lastTime = currentTime;

    // Calculate FPS
    m_fpsAccum += m_deltaTime;
    m_frameCount++;
    if (m_fpsAccum >= 1.0f) {
        m_fps = static_cast<float>(m_frameCount) / m_fpsAccum;
        m_frameCount = 0;
        m_fpsAccum = 0.0f;
    }
}

double Timer::getTime() const {
    auto now = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double>(now.time_since_epoch()).count();
}

} // namespace terrafirma

