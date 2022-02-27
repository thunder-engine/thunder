#ifndef ANIMATOR_H
#define ANIMATOR_H

#include "nativebehaviour.h"

class AnimationClip;
class AnimationStateMachine;

class AnimatorPrivate;

class ENGINE_EXPORT Animator : public NativeBehaviour {
    A_REGISTER(Animator, NativeBehaviour, Components/Animation)

    A_PROPERTIES(
        A_PROPERTYEX(AnimationStateMachine *, stateMachine, Animator::stateMachine, Animator::setStateMachine, "editor=Template")
    )

    A_METHODS(
        A_METHOD(void, Animator::setState),
        A_METHOD(void, Animator::setStateHash),
        A_METHOD(void, Animator::crossFade),
        A_METHOD(void, Animator::crossFadeHash),
        A_METHOD(void, Animator::setBool),
        A_METHOD(void, Animator::setBoolHash),
        A_METHOD(void, Animator::setFloat),
        A_METHOD(void, Animator::setFloatHash),
        A_METHOD(void, Animator::setInteger),
        A_METHOD(void, Animator::setIntegerHash),
        A_METHOD(int, Animator::duration)
    )

public:
    Animator();
    ~Animator();

    AnimationStateMachine *stateMachine() const;
    void setStateMachine(AnimationStateMachine *resource);

    uint32_t position() const;
    void setPosition(uint32_t position);

    void setState(const string &state);
    void setStateHash(int hash);

    void crossFade(const string &state, float duration);
    void crossFadeHash(int hash, float duration);

    void setBool(const string &name, bool value);
    void setBoolHash(int hash, bool value);

    void setFloat(const string &name, float value);
    void setFloatHash(int hash, float value);

    void setInteger(const string &name, int32_t value);
    void setIntegerHash(int hash, int32_t value);

    int duration() const;

    void setClip(AnimationClip *clip);

    void rebind();

private:
    void start() override;
    void update() override;

    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

private:
    AnimatorPrivate *p_ptr;

};

#endif // ANIMATOR_H
