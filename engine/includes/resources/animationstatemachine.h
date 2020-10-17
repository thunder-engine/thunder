#ifndef ANIMATIONSTATEMACHINE_H
#define ANIMATIONSTATEMACHINE_H

#include "resource.h"

#include "animationclip.h"

class AnimationStateMachinePrivate;

class NEXT_LIBRARY_EXPORT AnimationState {
public:
    enum {
        Base
    };

    struct NEXT_LIBRARY_EXPORT Transition {
        AnimationState *m_pTargetState;

        size_t m_ConditionHash;

        bool checkCondition (const Variant &value);
    };
    typedef vector<Transition> TransitionArray;

    AnimationState();

    uint8_t type() const;

    TransitionArray m_Transitions;

    int m_Hash;

    AnimationClip *m_pClip;

    bool m_Loop;
};
typedef vector<AnimationState *> AnimationStateVector;

class NEXT_LIBRARY_EXPORT AnimationStateMachine : public Resource {
    A_REGISTER(AnimationStateMachine, Resource, Resources)

public:
    typedef unordered_map<int, Variant> VariableMap;

public:
    AnimationStateMachine();

    AnimationState *findState(int hash) const;

    AnimationState *initialState() const;

    AnimationStateVector &states() const;

    VariableMap &variables() const;

private:
    void loadUserData(const VariantMap &data) override;

private:
    AnimationStateMachinePrivate *p_ptr;

};

#endif // ANIMATIONSTATEMACHINE_H
