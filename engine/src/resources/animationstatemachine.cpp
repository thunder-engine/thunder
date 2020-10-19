#include "resources/animationstatemachine.h"

#define MACHINE "Machine"

static hash<string> hash_str;

class AnimationStateMachinePrivate {
public:
    AnimationStateMachinePrivate() :
            m_pInitialState(nullptr) {

    }

    AnimationStateVector m_States;

    AnimationState *m_pInitialState;

    AnimationStateMachine::VariableMap m_Variables;
};

AnimationState::AnimationState() :
        m_Hash(0),
        m_pClip(nullptr),
        m_Loop(false) {

}

bool AnimationState::operator== (const AnimationState &right) const {
    return m_Hash == right.m_Hash;
}

bool AnimationTransition::operator== (const AnimationTransition &right) const {
    return m_ConditionHash == right.m_ConditionHash;
}

bool AnimationTransition::checkCondition(const Variant &value) {
    return value.toBool();
}
/*!
    Returns a type of state.
    For more details please see the AnimationState::Type enum.
*/
int AnimationState::type() const {
    return Base;
}

AnimationStateMachine::AnimationStateMachine() :
         p_ptr(new AnimationStateMachinePrivate) {

}
/*!
    \internal
*/
void AnimationStateMachine::loadUserData(const VariantMap &data) {
    PROFILE_FUNCTION();

    p_ptr->m_States.clear();
    p_ptr->m_Variables.clear();

    auto section = data.find(MACHINE);
    if(section != data.end()) {
        VariantList machine = (*section).second.value<VariantList>();
        if(machine.size() >= 3) {
            auto block = machine.begin();
            // Unpack states
            for(auto it : (*block).value<VariantList>()) {
                VariantList stateList = it.toList();
                auto i = stateList.begin();

                AnimationState *state = nullptr;
                switch((*i).toInt()) {
                    default: state = new AnimationState;
                }
                i++;
                state->m_Hash = hash_str((*i).toString());
                i++;
                state->m_pClip = Engine::loadResource<AnimationClip>((*i).toString());
                i++;
                state->m_Loop = (*i).toBool();

                p_ptr->m_States.push_back(state);
            }
            block++;
            // Unpack variables
            for(auto it : (*block).toMap()) {
                p_ptr->m_Variables[hash_str(it.first)] = it.second;
            }
            block++;
            // Unpack transitions
            for(auto it : (*block).value<VariantList>()) {
                VariantList valueList = it.toList();
                auto i = valueList.begin();

                AnimationState *source = findState(hash_str((*i).toString()));
                if(source) {
                    i++;
                    AnimationState *target = findState(hash_str((*i).toString()));
                    if(target) {
                        AnimationTransition transition;
                        transition.m_pTargetState = target;
                        source->m_Transitions.push_back(transition);
                    }
                }
            }
            block++;
            p_ptr->m_pInitialState = findState(hash_str((*block).toString()));
        }
    }

    setState(Ready);
}
/*!
    Returns a state for the provided \a hash.
*/
AnimationState *AnimationStateMachine::findState(int hash) const {
    PROFILE_FUNCTION();

    for(auto state : p_ptr->m_States) {
        if(state->m_Hash == hash) {
            return state;
        }
    }
    return nullptr;
}
/*!
    Returns an initial state for the state machine.
*/
AnimationState *AnimationStateMachine::initialState() const {
    PROFILE_FUNCTION();

    return p_ptr->m_pInitialState;
}
/*!
    \internal
    Returns an array of all states for the state machine.
*/
AnimationStateVector &AnimationStateMachine::states() const {
    PROFILE_FUNCTION();

    return p_ptr->m_States;
}
/*!
    \internal
    Sets a new \a value with the given \a name for the state machine.
    This variable can be used for transition cases between states.
*/
void AnimationStateMachine::setVariable(const string &name, const Variant &value) {
    PROFILE_FUNCTION();

    p_ptr->m_Variables[hash_str(name)] = value;
}
/*!
    \internal
    Returns a dictionary of all variables for the state machine.
*/
AnimationStateMachine::VariableMap &AnimationStateMachine::variables() const {
    PROFILE_FUNCTION();

    return p_ptr->m_Variables;
}

void AnimationStateMachine::registerSuper(ObjectSystem *system) {
    REGISTER_META_TYPE(AnimationState);
    REGISTER_META_TYPE(AnimationTransition);
    AnimationStateMachine::registerClassFactory(system);
}
