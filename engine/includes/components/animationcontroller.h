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
    AnimationController ();
    ~AnimationController ();

    AnimationStateMachine *stateMachine () const;
    void setStateMachine (AnimationStateMachine *machine);

    uint32_t position () const;
    void setPosition (uint32_t ms);

    void setState (string &state);
    void setState (size_t hash);

    AnimationClip *clip ();
    void setClip (AnimationClip *clip);

    uint32_t duration () const;

private:
    void start () override;
    void update () override;

    void loadUserData (const VariantMap &data) override;
    VariantMap saveUserData () const override;

    Object *findTarget (Object *src, const string &path);

private:
    AnimationControllerPrivate *p_ptr;

};

#endif // ANIMATIONCONTROLLER_H
