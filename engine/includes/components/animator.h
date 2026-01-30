#ifndef ANIMATOR_H
#define ANIMATOR_H

#include <nativebehaviour.h>

#include <animationstatemachine.h>

class AnimationClip;
class AnimationState;
class AnimationStateMachine;

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
        A_METHOD(void, Animator::setIntegerHash)
    )

    struct PlaybackState {
        Motion *motion = nullptr;

        AnimationState *state = nullptr;

        float currentPosition = 0.0f;

        float weight = 1.0f;

    };

    class TargetProperties {
    public:
        TargetProperties() :
            property(nullptr) {

        }

        std::list<PlaybackState> playbacks;

        Variant defaultValue;

        MetaProperty property;

        Object *object = nullptr;

        int flag = 0;

    };

public:
    Animator();
    ~Animator();

    AnimationStateMachine *stateMachine() const;
    void setStateMachine(AnimationStateMachine *machine);

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

    void setClip(AnimationClip *clip, float position = -1.0f);

    void rebind();

private:
    void update() override;

    void checkNextState();

    void checkEndOfTransition();

    bool recalcTransitionWeights(PlaybackState &playback, float position, float &factor) const;

    bool updatePosition(PlaybackState &playback, float position) const;

    void process(float dt);

    void sampleVector4(float dt, TargetProperties &target);

    void sampleQuaternion(float dt, TargetProperties &target);

    void sampleString(float dt, TargetProperties &target);

    static void stateMachineUpdated(int state, void *ptr);

private:
    friend class AnimatorTest;

    std::unordered_map<int32_t, Variant> m_currentVariables;

    std::unordered_map<uint32_t, TargetProperties> m_bindProperties;

    AnimationStateMachine *m_stateMachine;

    AnimationState *m_currentState;

    AnimationClip *m_currentClip;

    float m_transitionDuration;

};

#endif // ANIMATOR_H
