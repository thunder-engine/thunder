#ifndef ANIMATIONSTATEMACHINE_H
#define ANIMATIONSTATEMACHINE_H

#include "engine.h"

#include "animationclip.h"

class NEXT_LIBRARY_EXPORT AnimationStateMachine : public Object {
    A_REGISTER(AnimationStateMachine, Object, Resources)

public:
    class NEXT_LIBRARY_EXPORT State {
    public:
        enum {
            Base
        };

        struct NEXT_LIBRARY_EXPORT Transition {
            State *m_pTargetState;

            size_t m_ConditionHash;
        };
        typedef vector<Transition> TransitionArray;

        State();

        uint8_t type() const;

        TransitionArray m_Transitions;

        size_t m_Hash;

        AnimationClip *m_pClip;

        bool m_Loop;
    };
    typedef vector<State *> StateVector;

    typedef unordered_map<size_t, Variant> VariableMap;

public:
    AnimationStateMachine();

    void loadUserData(const VariantMap &data);

    State *findState(size_t hash) const;
public:
    StateVector m_States;

    State *m_pInitialState;

    VariableMap m_Variables;
};

#endif // ANIMATIONSTATEMACHINE_H
