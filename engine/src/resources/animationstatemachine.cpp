#include "resources/animationstatemachine.h"

namespace {
    const char *gMachine("Machine");
}

bool AnimationTransition::checkCondition(const Variant &value) {
    switch (m_compareRule) {
        case Equals: {
            return value == m_compareValue;
        } break;
        case NotEquals: {
            return value != m_compareValue;
        } break;
        case Greater: {
            switch(m_compareValue.type()) {
                case MetaType::BOOLEAN: return value.toBool() > m_compareValue.toBool();
                case MetaType::INTEGER: return value.toInt() > m_compareValue.toInt();
                case MetaType::FLOAT: return value.toFloat() > m_compareValue.toFloat();
                default: break;
            }
        } break;
        case Less: {
            switch(m_compareValue.type()) {
                case MetaType::BOOLEAN: return value.toBool() < m_compareValue.toBool();
                case MetaType::INTEGER: return value.toInt() < m_compareValue.toInt();
                case MetaType::FLOAT: return value.toFloat() < m_compareValue.toFloat();
                default: break;
            }
        } break;
        default: break;
    }
    return false;
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
        if(machine.size() >= 4) {
            auto block = machine.begin();
            // Unpack states
            for(auto &it : (*block).value<VariantList>()) {
                VariantList stateList = it.toList();
                auto i = stateList.begin();

                AnimationState *state = nullptr;
                TString type = (*i).toString();
                i++;
                if(type == "BaseState") {
                    state = new AnimationState;
                    state->m_hash = Mathf::hashString((*i).toString());
                    i++;
                    state->m_clip = Engine::loadResource<AnimationClip>((*i).toString());
                    i++;
                    state->m_loop = (*i).toBool();

                    m_states.push_back(state);
                }
            }
            block++;
            // Unpack variables
            for(auto &it : (*block).value<VariantMap>()) {
                m_variables[Mathf::hashString(it.first)] = it.second;
            }
            block++;
            // Unpack transitions
            for(auto &it : (*block).value<VariantList>()) {
                VariantList valueList = it.toList();
                auto i = valueList.begin();

                AnimationState *source = findState(Mathf::hashString((*i).toString()));
                if(source) {
                    i++;
                    AnimationState *target = findState(Mathf::hashString((*i).toString()));
                    if(target) {
                        AnimationTransition transition;
                        transition.m_targetState = target;
                        i++;
                        if(i != valueList.end()) {
                            transition.m_compareRule = (*i).toInt();
                            i++;
                            transition.m_compareValue = (*i);
                        } else {
                            transition.m_compareRule = AnimationTransition::Equals;
                            transition.m_compareValue = true;
                        }

                        source->m_transitions.push_back(transition);
                    }
                }
            }
            block++;
            m_initialState = findState(Mathf::hashString((*block).toString()));
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
