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
        A_PROPERTY(Direction, Direction, Animation::direction, Animation::setDirection),
        A_PROPERTY(uint32_t, Time, Animation::currentTime, Animation::setCurrentTime)
    )

public:
    enum Direction {
        FORWARD,
        BACKWARD
    };

    enum State {
        STOPPED,
        RUNNING,
        PAUSED
    };

public:
    Animation                       ();

    ~Animation                      ();

    Direction                       direction                   () const;
    void                            setDirection                (Direction direction);

    int32_t                         currentTime                 () const;
    void                            setCurrentTime              (int32_t msecs);

    int32_t                         loopCount                   () const;
    void                            setLoopCount                (int32_t loops);

    uint32_t                        currentLoop                 () const;

    uint32_t                        loopTime                    () const;

    State                           state                       () const;

    virtual int32_t                 loopDuration                () const;

    int32_t                         duration                    () const;

public:
    void                            start                       ();

    void                            stop                        ();

    void                            pause                       ();

    void                            resume                      ();

protected:
    virtual void                    update                      ();

private:
    AnimationPrivate               *p_ptr;
};

#endif // ANIMATION_H
