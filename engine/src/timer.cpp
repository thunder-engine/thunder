#include "timer.h"

static TimePoint m_sLastTime;
static float m_sFixedDelta  = 0.0;
static float m_sTime        = 0.0;
static float m_sDeltaTime   = 0.0;
static float m_sTimeScale   = 1.0;

void Timer::init(float fixed) {
    m_sLastTime     = std::chrono::high_resolution_clock::now();
    m_sFixedDelta   = fixed;
}

void Timer::reset() {
    m_sFixedDelta  = 0.0;
    m_sTime        = 0.0;
    m_sDeltaTime   = 0.0;
    m_sTimeScale   = 1.0;
}

void Timer::update() {
    TimePoint current   = std::chrono::high_resolution_clock::now();

    m_sDeltaTime    = (std::chrono::duration_cast<std::chrono::duration<float> >(current - m_sLastTime)).count() * m_sTimeScale;
    m_sTime        += m_sDeltaTime;
    m_sLastTime     = current;
}

float Timer::time() {
    return m_sTime;
}

float Timer::deltaTime() {
    return m_sDeltaTime;
}

float Timer::fixedDelta() {
    return m_sFixedDelta;
}

float Timer::scale() {
    return m_sTimeScale;
}

void Timer::setScale(float scale) {
    m_sTimeScale    = scale;
}
