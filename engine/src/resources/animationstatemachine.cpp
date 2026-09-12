#include "resources/animationstatemachine.h"

namespace {
    const char *gMachine("Machine");

    int stateHash(const Variant &value) {
        return value.type() == MetaType::INTEGER ? value.toInt() : Mathf::hashString(value.toString());
    }
}

bool AnimationTransitionCondition::check(const Variant &value) {
    switch(m_rule) {
        case Equals: return value == m_value;
        case NotEquals: return value != m_value;
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
                    state->m_hash = stateHash(*i);
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
                int hash = Mathf::hashString(it.first);
                if(TString::number(it.first.toInt()) == it.first) {
                    hash = it.first.toInt();
                }
                m_variables[hash] = it.second;
            }
            block++;
            // Unpack transitions
            for(auto &it : (*block).value<VariantList>()) {
                VariantList valueList(it.value<VariantList>());
                auto i = valueList.begin();

                AnimationState *source = findState(stateHash(*i));
                if(source) {
                    i++;
                    AnimationState *target = findState(stateHash(*i));
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
            m_initialState = findState(stateHash(*block));

            switchState(ToBeUpdated);
        }
    }
}
/*!
    \internal
*/
AnimationTransitionCondition AnimationStateMachine::loadCondition(const VariantList &data) const {
    AnimationTransitionCondition condition;

    auto it = data.begin();
    condition.m_hash = stateHash(*it);
    ++it;
    condition.m_rule = it->toInt();
    ++it;
    condition.m_value = *it;

    return condition;
}
/*!
    \internal
*/
VariantMap AnimationStateMachine::saveUserData() const {
    VariantMap result;

    VariantList states;
    for(const AnimationState *state : m_states) {
        VariantList data;
        data.push_back(TString("BaseState"));
        data.push_back(state->m_hash);
        data.push_back(state->m_clip ? Engine::reference(state->m_clip) : TString());
        data.push_back(state->m_loop);
        states.push_back(data);
    }

    VariantMap variables;
    for(const auto &variable : m_variables) {
        variables[TString::number(variable.first)] = variable.second;
    }

    VariantList transitions;
    for(const AnimationState *state : m_states) {
        for(const AnimationTransition &transition : state->m_transitions) {
            VariantList data;
            data.push_back(state->m_hash);
            data.push_back(transition.m_targetState ? transition.m_targetState->m_hash : 0);
            data.push_back(transition.m_duration);

            if(!transition.m_conditions.empty()) {
                VariantList conditions;
                for(const AnimationTransitionCondition &condition : transition.m_conditions) {
                    VariantList conditionData;
                    conditionData.push_back(condition.m_hash);
                    conditionData.push_back(condition.m_rule);
                    conditionData.push_back(condition.m_value);
                    conditions.push_back(conditionData);
                }
                data.push_back(conditions);
            }
            transitions.push_back(data);
        }
    }

    VariantList machine;
    machine.push_back(states);
    machine.push_back(variables);
    machine.push_back(transitions);
    machine.push_back(m_initialState ? m_initialState->m_hash : 0);
    result[gMachine] = machine;

    return result;
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
