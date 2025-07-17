#ifndef ANIMATOR_H
#define ANIMATOR_H

#include "nativebehaviour.h"

#include "animationstatemachine.h"

class AnimationClip;
class AnimationStateMachine;
class AnimationState;
class BaseAnimationBlender;

typedef std::unordered_map<int, Variant> VariableMap;

class ENGINE_EXPORT Animator : public NativeBehaviour {
    A_OBJECT(Animator, NativeBehaviour, Components/Animation)

    A_PROPERTIES(
        A_PROPERTYEX(AnimationStateMachine *, stateMachine, Animator::stateMachine, Animator::setStateMachine, "editor=Asset")
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
    void setStateMachine(AnimationStateMachine *machine);

    uint32_t position() const;
    void setPosition(uint32_t position);

    void setState(const TString &state);
    void setStateHash(int hash);

    void crossFade(const TString &state, float duration);
    void crossFadeHash(int hash, float duration);

    void setBool(const TString &name, bool value);
    void setBoolHash(int hash, bool value);

    void setFloat(const TString &name, float value);
    void setFloatHash(int hash, float value);

    void setInteger(const TString &name, int32_t value);
    void setIntegerHash(int hash, int32_t value);

    int duration() const;

    void setClip(AnimationClip *clip);

    void rebind();

    void resume();

private:
    void start() override;
    void update() override;

    void setClips(AnimationClip *start, AnimationClip *end, float duration = 0.0f, float time = 0.0f);

    static void stateMachineUpdated(int state, void *ptr);

private:
    std::unordered_map<uint32_t, BaseAnimationBlender *> m_properties;

    VariableMap m_currentVariables;

    AnimationStateMachine *m_stateMachine;

    AnimationState *m_currentState;

    AnimationClip *m_currentClip;

    uint32_t m_time;

};

#endif // ANIMATOR_H
