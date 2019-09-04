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

    void setState (const char *state);
    void setState (size_t hash);

    void crossFade (const char *state, float duration);
    void crossFade (size_t hash, float duration);

    AnimationClip *clip () const;
    void setClip (AnimationClip *clip);

    void setBool (const char *name, bool value);
    void setBool (size_t hash, bool value);

    void setFloat (const char *name, float value);
    void setFloat (size_t hash, float value);

    void setInteger (const char *name, int32_t value);
    void setInteger (size_t hash, int32_t value);

    uint32_t duration () const;

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
