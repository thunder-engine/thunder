#ifndef ANIMATION_H
#define ANIMATION_H

#include "core/objectsystem.h"

class AnimationPrivate;

class NEXT_LIBRARY_EXPORT Animation : public Object {
    A_REGISTER(Animation, Object, Animation)

    A_METHODS(
        A_SLOT(Animation::start),
        A_SLOT(Animation::stop),
        A_SLOT(Animation::pause),
        A_SLOT(Animation::resume)
    )

    A_PROPERTIES(
        A_PROPERTY(uint32_t, Time, Animation::currentTime, Animation::setCurrentTime)
    )

public:
    enum State {
        STOPPED,
        RUNNING,
        PAUSED
    };

public:
    Animation();

    ~Animation();

    uint32_t currentTime() const;
    virtual void setCurrentTime(uint32_t msecs);

    int32_t loopCount() const;
    void setLoopCount(int32_t loops);

    uint32_t currentLoop() const;

    uint32_t loopTime() const;

    State state() const;

    virtual int32_t duration() const;

    int32_t totalDuration() const;

    bool isValid() const;
    virtual void setValid(bool valid);

public:
    void start();

    void stop();

    void pause();

    void resume();

private:
    Animation::State m_state;

    uint32_t m_currentLoop;

    uint32_t m_currentTime;

    uint32_t m_totalCurrentTime;

    int32_t  m_loopCount;

    bool m_valid;

};

#endif // ANIMATION_H
