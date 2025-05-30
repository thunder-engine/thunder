#ifndef ANIMATIONSTATEMACHINE_H
#define ANIMATIONSTATEMACHINE_H

#include "resource.h"

#include "animationclip.h"

class AnimationState;

class ENGINE_EXPORT AnimationTransition {
public:
    bool operator== (const AnimationTransition &right) const;

    bool checkCondition(const Variant &value);

    AnimationState *m_targetState;

    int m_conditionHash;
};
typedef std::vector<AnimationTransition> TransitionVector;

class ENGINE_EXPORT AnimationState {
public:
    AnimationState();

    bool operator== (const AnimationState &right) const;

    TransitionVector m_transitions;

    int m_hash;

    AnimationClip *m_clip;

    bool m_loop;
};
typedef std::vector<AnimationState *> AnimationStateVector;

class ENGINE_EXPORT AnimationStateMachine : public Resource {
    A_OBJECT(AnimationStateMachine, Resource, Resources)

    A_METHODS(
        A_METHOD(AnimationState *, AnimationStateMachine::findState),
        A_METHOD(AnimationState *, AnimationStateMachine::initialState)
    )
    A_NOPROPERTIES()

public:
    typedef std::unordered_map<int, Variant> VariableMap;

public:
    AnimationStateMachine();

    AnimationState *findState(int hash) const;

    AnimationState *initialState() const;

    void setVariable(const std::string &name, const Variant &value);

    const AnimationStateVector &states() const;

    const VariableMap &variables() const;

    static void registerSuper(ObjectSystem *system);

private:
    void loadUserData(const VariantMap &data) override;

private:
    AnimationStateVector m_states;

    AnimationState *m_initialState;

    AnimationStateMachine::VariableMap m_variables;

};

#endif // ANIMATIONSTATEMACHINE_H
