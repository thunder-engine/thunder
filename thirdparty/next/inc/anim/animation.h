#ifndef ANIMATION_H
#define ANIMATION_H

#include "core/objectsystem.h"

class AnimationPrivate;

class NEXT_LIBRARY_EXPORT Animation : public Object {
    A_REGISTER(Animation, Object, Animation)

    A_METHODS(
        A_SLOT(start),
        A_SLOT(stop),
        A_SLOT(pause),
        A_SLOT(resume),
        A_SIGNAL(finished)
    )

    A_PROPERTIES(
        A_PROPERTY(Direction, Direction, direction, setDirection),
        A_PROPERTY(uint32_t, Time, currentTime, setCurrentTime)
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

    Direction                       direction                   () const;
    void                            setDirection                (Direction value);

    int32_t                         currentTime                 () const;
    void                            setCurrentTime              (int32_t msecs);

    int32_t                         loopCount                   () const;
    void                            setLoopCount                (int32_t loop);

    uint32_t                        currentLoop                 () const;

    uint32_t                        loopTime                    () const;

    State                           state                       () const;

    virtual int32_t                 loopDuration                () const;

    int32_t                         duration                    () const;

public:
    void                            finished                    ();

public:
    void                            start                       ();

    void                            stop                        ();

    void                            pause                       ();

    void                            resume                      ();

protected:
    bool                            event                       (Event *event);

    virtual void                    update                      ();

private:
    AnimationPrivate               *p_ptr;
};

#endif // ANIMATION_H
