#ifndef ANIMATIONSTATEMACHINE_H
#define ANIMATIONSTATEMACHINE_H

#include "resource.h"

#include "animationclip.h"

class AnimationState;

class ENGINE_EXPORT AnimationTransitionCondition {
public:
    enum CompareRules {
        Equals = 0,
        NotEquals,
        Greater,
        Less
    };

public:
    bool check(const Variant &value);

    bool operator== (const AnimationTransitionCondition &right) const {
        return m_hash == right.m_hash;
    }

    Variant m_value;

    int m_hash = 0;

    int m_rule = 0;

};

class ENGINE_EXPORT AnimationTransition {
public:
    bool operator== (const AnimationTransition &right) const {
        return m_targetState == right.m_targetState;
    }

    std::vector<AnimationTransitionCondition> m_conditions;

    AnimationState *m_targetState = nullptr;

    float m_duration = 0.0f;

};
typedef std::vector<AnimationTransition> TransitionVector;

class ENGINE_EXPORT AnimationState {
public:
    bool operator== (const AnimationState &right) const {
        return m_hash == right.m_hash;
    }

    TransitionVector m_transitions;

    AnimationClip *m_clip = nullptr;

    int m_hash = 0;

    bool m_loop = false;

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

    const AnimationStateVector &states() const;

    const VariableMap &variables() const;

    static void registerSuper(ObjectSystem *system);

private:
    void loadUserData(const VariantMap &data) override;

    AnimationTransitionCondition loadCondition(const VariantList &data) const;

private:
    friend class AnimatorTest;

    AnimationStateVector m_states;

    AnimationStateMachine::VariableMap m_variables;

    AnimationState *m_initialState;

};

#endif // ANIMATIONSTATEMACHINE_H
