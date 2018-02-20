#include "timer.h"

static TimePoint m_sLastTime;
static double m_sFixedDelta = 0.0;
static double m_sTime       = 0.0;
static double m_sDeltaTime  = 0.0;
static double m_sTimeScale  = 1.0;

void Timer::init(double fixed) {
    m_sLastTime     = std::chrono::high_resolution_clock::now();
    m_sFixedDelta   = fixed;
}

void Timer::update() {
    TimePoint current   = std::chrono::high_resolution_clock::now();

    m_sDeltaTime    = (std::chrono::duration_cast<std::chrono::duration<double> >(current - m_sLastTime)).count();
    m_sTime        += m_sDeltaTime;
    m_sLastTime     = current;
}

double Timer::time() {
    return m_sTime;
}

double Timer::deltaTime() {
    return m_sDeltaTime;
}

double Timer::fixedDelta() {
    return m_sFixedDelta;
}

double Timer::scale() {
    return m_sTimeScale;
}

void Timer::setScale(double scale) {
    m_sTimeScale    = scale;
}
