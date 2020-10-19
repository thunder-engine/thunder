#ifndef ANIMATIONSTATEMACHINE_H
#define ANIMATIONSTATEMACHINE_H

#include "resource.h"

#include "animationclip.h"

class AnimationStateMachinePrivate;

class AnimationState;

class NEXT_LIBRARY_EXPORT AnimationTransition {
public:
    bool operator== (const AnimationTransition &right) const;

    bool checkCondition (const Variant &value);

    AnimationState *m_pTargetState;

    int m_ConditionHash;
};
typedef vector<AnimationTransition> TransitionVector;

class NEXT_LIBRARY_EXPORT AnimationState {
public:
    enum Type {
        Base
    };

    AnimationState();

    bool operator== (const AnimationState &right) const;

    int type() const;

    TransitionVector m_Transitions;

    int m_Hash;

    AnimationClip *m_pClip;

    bool m_Loop;
};
typedef vector<AnimationState *> AnimationStateVector;

class NEXT_LIBRARY_EXPORT AnimationStateMachine : public Resource {
    A_REGISTER(AnimationStateMachine, Resource, Resources)

    A_METHODS(
        A_METHOD(AnimationState *, AnimationStateMachine::findState),
        A_METHOD(AnimationState *, AnimationStateMachine::initialState)
    )

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
