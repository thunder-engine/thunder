/*
    This file is part of Thunder Engine.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "timer.h"

static TimePoint m_sLastTime;
static float m_sTime      = 0.0;
static float m_sDeltaTime = 0.0;
static float m_sTimeScale = 1.0;

/*!
    \class Timer
    \brief The interface to get time information from Thunder Engine.
    \inmodule Engine

    The Timer class helps to make your game more smooth and accurate.
    This class is used in all systems which doing any animation.
    Using deltaTime() method developers are able to calculate a logic based on delays for example shots or movements of your character.
    Time scale value can be used for the slow-motion effects because it applied for all deltaTime() values.
*/

/*!
    Resets all Timer related variables.
    \note Usually, this method calls internally and must not be called manually.
    \internal
*/
void Timer::reset() {
    m_sTime = 0.0;
    m_sDeltaTime = 0.0;
    m_sTimeScale = 1.0;
    m_sLastTime = std::chrono::high_resolution_clock::now();
}
/*!
    Updates all Timer related variables.
    \note Usually, this method calls internally and must not be called manually.
    \internal
*/
void Timer::update() {
    TimePoint current = std::chrono::high_resolution_clock::now();

    m_sDeltaTime = (std::chrono::duration_cast<std::chrono::duration<float> >(current - m_sLastTime)).count() * m_sTimeScale;
    m_sTime += m_sDeltaTime;
    m_sLastTime = current;
}
/*!
    Returns the time in seconds since the start of the game.
    \note This value is updated in each frame. In case of calling multiple times in a single frame will return the same result.
*/
float Timer::time() {
    return m_sTime;
}
/*!
    Returns the time in seconds since the last frame.
    \note This value is updated in each frame. In case of calling multiple times in a single frame will return the same result.
*/
float Timer::deltaTime() {
    return m_sDeltaTime;
}
/*!
    Return the time scale at which the time is passing.
*/
float Timer::scale() {
    return m_sTimeScale;
}
/*!
    Sets the time \a scale at which the time is passing.
*/
void Timer::setScale(float scale) {
    m_sTimeScale = scale;
}
