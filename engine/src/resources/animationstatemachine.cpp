#include "resources/animationstatemachine.h"

namespace {
    const char *gMachine = "Machine";
}

static std::hash<std::string> hash_str;

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
    \inmodule Resources
*/

AnimationStateMachine::AnimationStateMachine() :
         m_initialState(nullptr) {

}
/*!
    \internal
*/
void AnimationStateMachine::loadUserData(const VariantMap &data) {
    PROFILE_FUNCTION();

    m_states.clear();
    m_variables.clear();

    auto section = data.find(gMachine);
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
                std::string str = (*i).toString();
                state->m_hash = hash_str(str);
                i++;
                state->m_clip = Engine::loadResource<AnimationClip>((*i).toString());
                i++;
                state->m_loop = (*i).toBool();

                m_states.push_back(state);
            }
            block++;
            // Unpack variables
            for(auto &it : (*block).toMap()) {
                m_variables[hash_str(it.first)] = it.second;
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
            m_initialState = findState(hash_str((*block).toString()));
        }
    }
}
/*!
    Returns a state for the provided \a hash.
*/
AnimationState *AnimationStateMachine::findState(int hash) const {
    PROFILE_FUNCTION();

    for(auto state : m_states) {
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

    return m_initialState;
}
/*!
    \internal
    Returns an array of all states for the state machine.
*/
const AnimationStateVector &AnimationStateMachine::states() const {
    PROFILE_FUNCTION();

    return m_states;
}
/*!
    \internal
    Sets a new \a value with the given \a name for the state machine.
    This variable can be used for transition cases between states.
*/
void AnimationStateMachine::setVariable(const std::string &name, const Variant &value) {
    PROFILE_FUNCTION();

    m_variables[hash_str(name)] = value;
}
/*!
    \internal
    Returns a dictionary of all variables for the state machine.
*/
const AnimationStateMachine::VariableMap &AnimationStateMachine::variables() const {
    PROFILE_FUNCTION();

    return m_variables;
}
/*!
    \internal
*/
void AnimationStateMachine::registerSuper(ObjectSystem *system) {
    REGISTER_META_TYPE(AnimationState);
    REGISTER_META_TYPE(AnimationTransition);
    AnimationStateMachine::registerClassFactory(system);
}
