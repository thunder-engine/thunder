#ifndef ANIMATIONSTATEMACHINE_H
#define ANIMATIONSTATEMACHINE_H

#include "resource.h"

#include "animationclip.h"

class AnimationStateMachinePrivate;

class NEXT_LIBRARY_EXPORT AnimationStateMachine : public Resource {
    A_REGISTER(AnimationStateMachine, Resource, Resources)

public:
    class NEXT_LIBRARY_EXPORT State {
    public:
        enum {
            Base
        };

        struct NEXT_LIBRARY_EXPORT Transition {
            State *m_pTargetState;

            size_t m_ConditionHash;

            bool checkCondition (const Variant &value);
        };
        typedef vector<Transition> TransitionArray;

        State();

        uint8_t type() const;

        TransitionArray m_Transitions;

        size_t m_Hash;

        AnimationClip *m_pClip;

        bool m_Loop;
    };

    typedef unordered_map<size_t, Variant> VariableMap;

    typedef vector<AnimationStateMachine::State *> StateVector;

public:
    AnimationStateMachine();

    State *findState(size_t hash) const;

    State *initialState() const;

    StateVector &states() const;

    VariableMap &variables() const;

private:
    void loadUserData(const VariantMap &data) override;

private:
    AnimationStateMachinePrivate *p_ptr;

};

#endif // ANIMATIONSTATEMACHINE_H
