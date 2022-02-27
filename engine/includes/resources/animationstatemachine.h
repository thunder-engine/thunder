#ifndef ANIMATIONSTATEMACHINE_H
#define ANIMATIONSTATEMACHINE_H

#include "resource.h"

#include "animationclip.h"

class AnimationStateMachinePrivate;

class AnimationState;

class ENGINE_EXPORT AnimationTransition {
public:
    bool operator== (const AnimationTransition &right) const;

    bool checkCondition(const Variant &value);

    AnimationState *m_targetState;

    int m_conditionHash;
};
typedef vector<AnimationTransition> TransitionVector;

class ENGINE_EXPORT AnimationState {
public:
    AnimationState();

    bool operator== (const AnimationState &right) const;

    TransitionVector m_transitions;

    int m_hash;

    AnimationClip *m_clip;

    bool m_loop;
};
typedef vector<AnimationState *> AnimationStateVector;

class ENGINE_EXPORT AnimationStateMachine : public Resource {
    A_REGISTER(AnimationStateMachine, Resource, Resources)

    A_METHODS(
        A_METHOD(AnimationState *, AnimationStateMachine::findState),
        A_METHOD(AnimationState *, AnimationStateMachine::initialState)
    )
    A_NOPROPERTIES()

public:
    typedef unordered_map<int, Variant> VariableMap;

public:
    AnimationStateMachine();

    AnimationState *findState(int hash) const;

    AnimationState *initialState() const;

    void setVariable(const string &name, const Variant &value);

    AnimationStateVector &states() const;

    VariableMap &variables() const;

    static void registerSuper(ObjectSystem *system);

private:
    void loadUserData(const VariantMap &data) override;

private:
    AnimationStateMachinePrivate *p_ptr;

};

#endif // ANIMATIONSTATEMACHINE_H
