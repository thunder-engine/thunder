#ifndef ANIMATIONCONTROLLER_H
#define ANIMATIONCONTROLLER_H

#include "nativebehaviour.h"

class AnimationClip;
class AnimationStateMachine;

class AnimationControllerPrivate;

class NEXT_LIBRARY_EXPORT AnimationController : public NativeBehaviour {
    A_REGISTER(AnimationController, NativeBehaviour, Components)

    A_PROPERTIES (
        A_PROPERTYEX(AnimationStateMachine *, stateMachine, AnimationController::stateMachine, AnimationController::setStateMachine, "editor=Template"),
        A_PROPERTY(AnimationClip *, clip, AnimationController::clip, AnimationController::setClip)
    )

    A_METHODS(
        A_METHOD(void, AnimationController::setState),
        A_METHOD(void, AnimationController::setStateHash),
        A_METHOD(void, AnimationController::crossFade),
        A_METHOD(void, AnimationController::crossFadeHash),
        A_METHOD(void, AnimationController::setBool),
        A_METHOD(void, AnimationController::setBoolHash),
        A_METHOD(void, AnimationController::setFloat),
        A_METHOD(void, AnimationController::setFloatHash),
        A_METHOD(void, AnimationController::setInteger),
        A_METHOD(void, AnimationController::setIntegerHash),
        A_METHOD(int, AnimationController::duration)
    )

public:
    AnimationController ();
    ~AnimationController ();

    AnimationStateMachine *stateMachine () const;
    void setStateMachine (AnimationStateMachine *resource);

    uint32_t position () const;
    void setPosition (uint32_t position);

    void setState (const string &state);
    void setStateHash (int hash);

    void crossFade (string &state, float duration);
    void crossFadeHash (int hash, float duration);

    AnimationClip *clip () const;
    void setClip (AnimationClip *clip);

    void setBool (const string &name, bool value);
    void setBoolHash (int hash, bool value);

    void setFloat (const string &name, float value);
    void setFloatHash (int hash, float value);

    void setInteger (const string &name, int32_t value);
    void setIntegerHash (int hash, int32_t value);

    int duration () const;

private:
    void start () override;
    void update () override;

    void setClips (AnimationClip *start, AnimationClip *end, float duration = 0.0f, float time = 0.0f);

    void loadUserData (const VariantMap &data) override;
    VariantMap saveUserData () const override;

    Object *findTarget (Object *src, const string &path);

private:
    AnimationControllerPrivate *p_ptr;

};

#endif // ANIMATIONCONTROLLER_H
