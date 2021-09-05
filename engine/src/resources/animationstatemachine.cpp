#include "resources/animationstatemachine.h"

#define MACHINE "Machine"

static hash<string> hash_str;

class AnimationStateMachinePrivate {
public:
    AnimationStateMachinePrivate() :
            m_initialState(nullptr) {

    }

    AnimationStateVector m_states;

    AnimationState *m_initialState;

    AnimationStateMachine::VariableMap m_variables;
};

AnimationState::AnimationState() :
        m_hash(0),
        m_clip(nullptr),
        m_loop(false) {

}

bool AnimationState::operator==(const AnimationState &right) const {
    return m_hash == right.m_hash;
}

bool AnimationTransition::operator==(const AnimationTransition &right) const {
    return m_conditionHash == right.m_conditionHash;
}

bool AnimationTransition::checkCondition(const Variant &value) {
    return value.toBool();
}

/*!
    \class AnimationStateMachine
    \brief AnimationStateMachine resource contains information about animation states and transition rules.
    \inmodule Resource
*/

AnimationStateMachine::AnimationStateMachine() :
         p_ptr(new AnimationStateMachinePrivate) {

}
/*!
    \internal
*/
void AnimationStateMachine::loadUserData(const VariantMap &data) {
    PROFILE_FUNCTION();

    p_ptr->m_states.clear();
    p_ptr->m_variables.clear();

    auto section = data.find(MACHINE);
    if(section != data.end()) {
        VariantList machine = (*section).second.value<VariantList>();
        if(machine.size() >= 3) {
            auto block = machine.begin();
            // Unpack states
            for(auto &it : (*block).value<VariantList>()) {
                VariantList stateList = it.toList();
                auto i = stateList.begin();

                AnimationState *state = nullptr;
                switch((*i).toInt()) {
                    default: state = new AnimationState;
                }
                i++;
                string str = (*i).toString();
                state->m_hash = hash_str(str);
                i++;
                state->m_clip = Engine::loadResource<AnimationClip>((*i).toString());
                i++;
                state->m_loop = (*i).toBool();

                p_ptr->m_states.push_back(state);
            }
            block++;
            // Unpack variables
            for(auto &it : (*block).toMap()) {
                p_ptr->m_variables[hash_str(it.first)] = it.second;
            }
            block++;
            // Unpack transitions
            for(auto &it : (*block).value<VariantList>()) {
                VariantList valueList = it.toList();
                auto i = valueList.begin();

                AnimationState *source = findState(hash_str((*i).toString()));
                if(source) {
                    i++;
                    AnimationState *target = findState(hash_str((*i).toString()));
                    if(target) {
                        AnimationTransition transition;
                        transition.m_targetState = target;
                        source->m_transitions.push_back(transition);
                    }
                }
            }
            block++;
            p_ptr->m_initialState = findState(hash_str((*block).toString()));
        }
    }

    setState(Ready);
}
/*!
    Returns a state for the provided \a hash.
*/
AnimationState *AnimationStateMachine::findState(int hash) const {
    PROFILE_FUNCTION();

    for(auto state : p_ptr->m_states) {
        if(state->m_hash == hash) {
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

    return p_ptr->m_initialState;
}
/*!
    \internal
    Returns an array of all states for the state machine.
*/
AnimationStateVector &AnimationStateMachine::states() const {
    PROFILE_FUNCTION();

    return p_ptr->m_states;
}
/*!
    \internal
    Sets a new \a value with the given \a name for the state machine.
    This variable can be used for transition cases between states.
*/
void AnimationStateMachine::setVariable(const string &name, const Variant &value) {
    PROFILE_FUNCTION();

    p_ptr->m_variables[hash_str(name)] = value;
}
/*!
    \internal
    Returns a dictionary of all variables for the state machine.
*/
AnimationStateMachine::VariableMap &AnimationStateMachine::variables() const {
    PROFILE_FUNCTION();

    return p_ptr->m_variables;
}
/*!
    \internal
*/
void AnimationStateMachine::registerSuper(ObjectSystem *system) {
    REGISTER_META_TYPE(AnimationState);
    REGISTER_META_TYPE(AnimationTransition);
    AnimationStateMachine::registerClassFactory(system);
}
