#include "core/Timer.h"
#include <GLFW/glfw3.h>
void Timer::update() {
    float currentFrame = static_cast<float>(glfwGetTime());
    m_deltaTime = currentFrame - m_lastFrame;
    m_lastFrame = currentFrame;
    m_accumulator += m_deltaTime;
}
bool Timer::shouldStepPhysics() {
    if (m_accumulator >= m_fixedTimeStep) {
        m_accumulator -= m_fixedTimeStep;
        return true;
    }
    return false;
}
