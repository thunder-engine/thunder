/*
    This file is part of Thunder Next.

    Thunder Next is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

    Thunder Next is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Thunder Next.  If not, see <http://www.gnu.org/licenses/>.

    Copyright: 2008-2025 Evgeniy Prikazchikov
*/

#include "anim/animation.h"

/*!
    \module Animation

    \title Next Animation Module

    \brief Contains classes to work with animation.
*/

/*!
    \class Animation
    \brief The Animation class provides base class interface for animations.
    \since Next 1.0
    \inmodule Animation

    The Animation class contain basic state machine to control animation processing.
*/
/*!
    \enum Animation::State

    This enum defines the state of animation track.

    \value STOPPED \c Animation stopped if start() is triggered the animation will start from beginning.
    \value RUNNING \c Animation is playing.
    \value PAUSED \c Animation paused if resume() is triggered the animation will continue from place before pause().
*/

Animation::Animation() :
    m_state(Animation::STOPPED),
    m_currentLoop(0),
    m_currentTime(0),
    m_totalCurrentTime(0),
    m_loopCount(-1),
    m_valid(true) {

}

Animation::~Animation() {

}
/*!
    Returns the current time (in milliseconds) in scope of current loop.
*/
uint32_t Animation::currentTime() const {
    return m_totalCurrentTime;
}
/*!
    Sets the new position of animation to provided \a msecs position.
    \note If new position placed outside of current loop; Then current loop will be changed to appropriate.
*/
void Animation::setCurrentTime(uint32_t msecs) {
    if(m_state == RUNNING) {
        int32_t total = totalDuration();
        if(total > -1) {
            msecs = MIN(static_cast<uint32_t>(total), msecs);
        }
        m_totalCurrentTime = msecs;

        int32_t length = duration();
        m_currentLoop = ((length > 0) ? (msecs / static_cast<uint32_t>(length)) : 0);
        if(m_currentLoop == static_cast<uint32_t>(m_loopCount)) {
            m_currentTime = static_cast<uint32_t>(MAX(0, length));
            m_currentLoop = static_cast<uint32_t>(MAX(0, m_loopCount - 1));
        } else {
            m_currentTime = (length > 0) ? (msecs % static_cast<uint32_t>(length)) : msecs;
        }
        if(m_totalCurrentTime == static_cast<uint32_t>(total)) {
            stop();
        }
    }
}
/*!
    Returns the number of repetitions of animation; -1 in case of infinite animation.
*/
int32_t Animation::loopCount() const {
    return m_loopCount;
}
/*!
    Sets the new number of \a loops of animation; -1 in case of infinite animation.
*/
void Animation::setLoopCount(int32_t loops) {
    m_loopCount = loops;
}
/*!
    Returns the number of repetitions of animation which already has played.
*/
uint32_t Animation::currentLoop() const {
    return m_currentLoop;
}
/*!
    Returns the current time for the current loop (in milliseconds).
*/
uint32_t Animation::loopTime() const {
    return m_currentTime;
}
/*!
    Returns the current state of animation.
*/
Animation::State Animation::state() const {
    return m_state;
}
/*!
    Returns the duration of the animation (in milliseconds).
*/
int32_t Animation::duration() const {
    return -1;
}
/*!
    Returns the duration (in milliseconds) in total as sum of durations for all loops.
    \note Returns -1 in case of infinite animation.
*/
int32_t Animation::totalDuration() const {
    int32_t length = duration();
    if(length <= 0) {
        return length;
    }
    int32_t count = loopCount();
    if(count < 0) {
        return -1;
    }
    return length * count;
}
/*!
    Returns true in case of animation is valid; otherwise returns false.
*/
bool Animation::isValid() const {
    return m_valid;
}
/*!
    Sets the \a valid state of animation. The invalid animations will not affect anything.
*/
void Animation::setValid(bool valid) {
    m_valid = valid;
}

/*!
    Starts the animation from the beginning.
*/
void Animation::start() {
    if(m_state == RUNNING) {
        return;
    }
    m_state = RUNNING;
    m_currentLoop = 0;
    m_currentTime = 0;
    m_totalCurrentTime = 0;
    setCurrentTime(0);
}
/*!
    Stops the animation.
    \note Animation can't be continued.
*/
void Animation::stop() {
    m_state = STOPPED;
}
/*!
    Stops the animation.
    \note Animation CAN be continued by resume().
*/
void Animation::pause() {
    if(m_state == STOPPED) {
        return;
    }
    m_state = PAUSED;
}
/*!
    Continues the animation which was paused earlier.
    Flag \a ignore can help to skip pause check.
*/
void Animation::resume(bool ignore) {
    if(m_state == PAUSED || ignore) {
        m_state = RUNNING;
    }
}


