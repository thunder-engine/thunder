#include "resources/animationstatemachine.h"

namespace {
    const char *gMachine("Machine");
}

bool AnimationTransitionCondition::check(const Variant &value) {
    switch(m_rule) {
        case Equals: {
            return value == m_value;
        } break;
        case NotEquals: {
            return value != m_value;
        } break;
        case Greater: {
            switch(m_value.type()) {
                case MetaType::BOOLEAN: return value.toBool() > m_value.toBool();
                case MetaType::INTEGER: return value.toInt() > m_value.toInt();
                case MetaType::FLOAT: return value.toFloat() > m_value.toFloat();
                default: break;
            }
        } break;
        case Less: {
            switch(m_value.type()) {
                case MetaType::BOOLEAN: return value.toBool() < m_value.toBool();
                case MetaType::INTEGER: return value.toInt() < m_value.toInt();
                case MetaType::FLOAT: return value.toFloat() < m_value.toFloat();
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
                VariantList stateList(it.value<VariantList>());
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
                VariantList valueList(it.value<VariantList>());
                auto i = valueList.begin();

                AnimationState *source = findState(Mathf::hashString((*i).toString()));
                if(source) {
                    i++;
                    AnimationState *target = findState(Mathf::hashString((*i).toString()));
                    if(target) {
                        AnimationTransition transition;
                        transition.m_targetState = target;
                        i++;
                        transition.m_duration = i->toFloat();
                        i++;

                        if(i != valueList.end()) { // has condition
                            for(auto &condition : i->toList()) {
                                transition.m_conditions.push_back( loadCondition(condition.value<VariantList>()) );
                            }
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
    \internal
*/
AnimationTransitionCondition AnimationStateMachine::loadCondition(const VariantList &data) const {
    AnimationTransitionCondition condition;

    auto it = data.begin();
    condition.m_hash = Mathf::hashString(it->toString());
    ++it;
    condition.m_rule = it->toInt();
    ++it;
    condition.m_value = *it;

    return condition;
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
