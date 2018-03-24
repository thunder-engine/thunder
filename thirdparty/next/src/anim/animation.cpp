#include "anim/animation.h"

class AnimationPrivate {
public:
    AnimationPrivate() :
            m_Direction(Animation::FORWARD),
            m_State(Animation::STOPPED),
            m_CurrentLoop(0),
            m_CurrentTime(0),
            m_TotalCurrentTime(0),
            m_LoopCount(-1) {

    }

    Animation::Direction    m_Direction;
    Animation::State        m_State;
    int32_t                 m_CurrentLoop;
    int32_t                 m_CurrentTime;
    int32_t                 m_TotalCurrentTime;
    int32_t                 m_LoopCount;
};

Animation::Animation() :
        p_ptr(new AnimationPrivate()) {

}

bool Animation::event(Event *event) {
    return Object::event(event);
}

void Animation::update() {

}

Animation::Direction Animation::direction() const {
    return p_ptr->m_Direction;
}

void Animation::setDirection(Direction value) {
    p_ptr->m_Direction  = value;
}

int32_t Animation::currentTime() const {
    return p_ptr->m_TotalCurrentTime;
}

void Animation::setCurrentTime(int32_t msecs) {
    int32_t total   = duration();
    if(total != -1) {
        msecs = min(total, msecs);
    }
    p_ptr->m_TotalCurrentTime   = msecs;

    int32_t length          = loopDuration();
    p_ptr->m_CurrentLoop    = ((length > 0) ? (msecs / length) : 0);
    if(p_ptr->m_CurrentLoop == p_ptr->m_LoopCount) {
        p_ptr->m_CurrentTime = max(0, length);
        p_ptr->m_CurrentLoop = max(0, p_ptr->m_LoopCount - 1);
    } else {
        if(p_ptr->m_Direction == BACKWARD) {
            p_ptr->m_CurrentTime    = (length > 0) ? ((msecs - 1) % length) + 1 : msecs;
            if(p_ptr->m_CurrentTime == length) {
                p_ptr->m_CurrentLoop--;
            }
        } else {
            p_ptr->m_CurrentTime    = (length > 0) ? (msecs % length) : msecs;
        }
    }
    update();
    if((p_ptr->m_Direction == FORWARD  && p_ptr->m_TotalCurrentTime == total) ||
       (p_ptr->m_Direction == BACKWARD && p_ptr->m_TotalCurrentTime == 0)) {
        stop();
    }
}

int32_t Animation::loopCount() const {
    return p_ptr->m_LoopCount;
}

void Animation::setLoopCount(int32_t loop) {
    p_ptr->m_LoopCount  = loop;
}

uint32_t Animation::currentLoop() const {
    return p_ptr->m_CurrentLoop;
}

uint32_t Animation::loopTime() const {
    return p_ptr->m_CurrentTime;
}

Animation::State Animation::state() const {
    return p_ptr->m_State;
}

int32_t Animation::loopDuration() const {
    return -1;
}

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

void Animation::start() {
    if(p_ptr->m_State == RUNNING) {
        return;
    }
    p_ptr->m_State  = RUNNING;
    setCurrentTime(0);
}

void Animation::stop() {
    p_ptr->m_State  = STOPPED;
}

void Animation::pause() {
    if(p_ptr->m_State == STOPPED) {
        return;
    }
    p_ptr->m_State  = PAUSED;
}

void Animation::resume() {
    if(p_ptr->m_State != PAUSED) {
        return;
    }
    p_ptr->m_State  = RUNNING;
}


