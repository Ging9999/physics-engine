#pragma once

class Timer {
public:
    void  update();
    bool  shouldStepPhysics();
    float getDeltaTime()    const { return m_deltaTime; }
    float getFixedTimeStep() const { return m_fixedTimeStep; }

private:
    float m_deltaTime    = 0.0f;
    float m_lastFrame    = 0.0f;
    float m_accumulator  = 0.0f;
    float m_fixedTimeStep = 1.0f / 60.0f;
};