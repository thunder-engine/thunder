#ifndef ANIMATIONCONTROLLER_H
#define ANIMATIONCONTROLLER_H

#include "nativebehaviour.h"

class AnimationClip;
class AnimationStateMachine;

class AnimationControllerPrivate;

class NEXT_LIBRARY_EXPORT AnimationController : public NativeBehaviour {
    A_REGISTER(AnimationController, NativeBehaviour, Components)

    A_PROPERTIES (
        A_PROPERTY(AnimationStateMachine*, State_Machine, AnimationController::stateMachine, AnimationController::setStateMachine)
    )

public:
    AnimationController         ();

    ~AnimationController        ();

    void                        start               ();

    void                        update              ();

    AnimationStateMachine      *stateMachine        () const;

    void                        setStateMachine     (AnimationStateMachine *machine);

    uint32_t                    position            () const;

    void                        setPosition         (uint32_t ms);

    void                        setState            (string &state);

    void                        setState            (size_t hash);

    AnimationClip              *clip                ();

    void                        setClip             (AnimationClip *clip);

    uint32_t                    duration            () const;

    void                        loadUserData        (const VariantMap &data);

    VariantMap                  saveUserData        () const;

protected:
    Object                     *findTarget          (Object *src, const string &path);

    AnimationControllerPrivate *p_ptr;
};

#endif // ANIMATIONCONTROLLER_H
