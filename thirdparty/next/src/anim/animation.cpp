#include "anim/animation.h"

class AnimationPrivate {
public:
    AnimationPrivate() :
            m_State(Animation::STOPPED),
            m_CurrentLoop(0),
            m_CurrentTime(0),
            m_TotalCurrentTime(0),
            m_LoopCount(-1) {

    }

    Animation::State        m_State;
    uint32_t                m_CurrentLoop;
    uint32_t                m_CurrentTime;
    uint32_t                m_TotalCurrentTime;
    int32_t                 m_LoopCount;
};
/*!
    \class Animation
    \brief The Animation class provides base class interface for animations.
    \since Next 1.0
    \inmodule Anim

    The Animation class contain basic state machine to control animation processing.
*/
/*!
    \enum Animation::Direction

    This enum defines the direction of playing animation.

    \value FORWARD \c Animation is playing normaly.
    \value BACKWARD \c Animation is reverted.
*/
/*!
    \enum Animation::State

    This enum defines the state of animation track.

    \value STOPPED \c Animation stopped if start() is triggered the animation will start from beginning.
    \value RUNNING \c Animation is playing.
    \value PAUSED \c Animation paused if resume() is triggered the animation will continue from place before pause().
*/
/*!
    Constructs Animation object.
*/
Animation::Animation() :
        p_ptr(new AnimationPrivate()) {

}

Animation::~Animation() {
    delete p_ptr;
}
/*!
    Returns the current time (in milliseconds) in scope of current loop.
*/
uint32_t Animation::currentTime() const {
    return p_ptr->m_TotalCurrentTime;
}
/*!
    Sets the new position of animation to provided \a msecs position.
    \note If new position placed outside of current loop; Then current loop will be changed to appropriate.
*/
void Animation::setCurrentTime(uint32_t msecs) {
    int32_t total   = duration();
    if(total > -1) {
        msecs = min(static_cast<uint32_t>(total), msecs);
    }
    p_ptr->m_TotalCurrentTime = msecs;

    int32_t length = loopDuration();
    p_ptr->m_CurrentLoop = ((length > 0) ? (msecs / static_cast<uint32_t>(length)) : 0);
    if(p_ptr->m_CurrentLoop == static_cast<uint32_t>(p_ptr->m_LoopCount)) {
        p_ptr->m_CurrentTime = static_cast<uint32_t>(max(0, length));
        p_ptr->m_CurrentLoop = static_cast<uint32_t>(max(0, p_ptr->m_LoopCount - 1));
    } else {
        p_ptr->m_CurrentTime = (length > 0) ? (msecs % static_cast<uint32_t>(length)) : msecs;
    }
    if(p_ptr->m_TotalCurrentTime == static_cast<uint32_t>(total)) {
        stop();
    }
}
/*!
    Returns the number of repetitions of animation; -1 in case of infinite animation.
*/
int32_t Animation::loopCount() const {
    return p_ptr->m_LoopCount;
}
/*!
    Sets the new number of \a loops of animation.
*/
void Animation::setLoopCount(int32_t loops) {
    p_ptr->m_LoopCount  = loops;
}
/*!
    Returns the number of repetitions of animation which already has played.
*/
uint32_t Animation::currentLoop() const {
    return p_ptr->m_CurrentLoop;
}
/*!
    Returns the current time for the current loop (in milliseconds).
*/
uint32_t Animation::loopTime() const {
    return p_ptr->m_CurrentTime;
}
/*!
    Returns the current state of animation.
*/
Animation::State Animation::state() const {
    return p_ptr->m_State;
}
/*!
    Returns the duration of the animation (in milliseconds).
*/
int32_t Animation::loopDuration() const {
    return -1;
}
/*!
    Returns the duration (in milliseconds) in total as sum of durations for all loops.
    \note Returns -1 in case of infinite animation.
*/
int32_t Animation::duration() const {
    int32_t length  = loopDuration();
    if(length <= 0) {
        return length;
    }
    int32_t count   = loopCount();
    if(count < 0) {
        return -1;
    }
    return length * count;
}
/*!
    Starts the animation from the beginning.
*/
void Animation::start() {
    if(p_ptr->m_State == RUNNING) {
        return;
    }
    p_ptr->m_State  = RUNNING;
    setCurrentTime(0);
}
/*!
    Stops the animation.
    \note Animation can't be continued.
*/
void Animation::stop() {
    p_ptr->m_State  = STOPPED;
}
/*!
    Stops the animation.
    \note Animation CAN be continued by resume().
*/
void Animation::pause() {
    if(p_ptr->m_State == STOPPED) {
        return;
    }
    p_ptr->m_State  = PAUSED;
}
/*!
    Continues the animation which was paused earlier.
*/
void Animation::resume() {
    if(p_ptr->m_State != PAUSED) {
        return;
    }
    p_ptr->m_State  = RUNNING;
}


