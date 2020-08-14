#include "resources/animationstatemachine.h"

#define MACHINE "Machine"

static hash<string> hash_str;

class AnimationStateMachinePrivate {
public:
    AnimationStateMachinePrivate() :
            m_pInitialState(nullptr) {

    }

    AnimationStateMachine::StateVector m_States;

    AnimationStateMachine::State *m_pInitialState;

    AnimationStateMachine::VariableMap m_Variables;
};

AnimationStateMachine::State::State() :
        m_Hash(0),
        m_pClip(nullptr),
        m_Loop(false) {

}

bool AnimationStateMachine::State::Transition::checkCondition(const Variant &value) {
    return value.toBool();
}

uint8_t AnimationStateMachine::State::type() const {
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

                State *state = nullptr;
                switch((*i).toInt()) {
                    default: state = new State;
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

                State *source = findState(hash_str((*i).toString()));
                if(source) {
                    i++;
                    State *target = findState(hash_str((*i).toString()));
                    if(target) {
                        State::Transition transition;
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

AnimationStateMachine::State *AnimationStateMachine::findState(size_t hash) const {
    PROFILE_FUNCTION();

    for(auto state : p_ptr->m_States) {
        if(state->m_Hash == hash) {
            return state;
        }
    }
    return nullptr;
}

AnimationStateMachine::State *AnimationStateMachine::initialState() const {
    PROFILE_FUNCTION();

    return p_ptr->m_pInitialState;
}
/*!
    Return list of all states for the state machine.
*/
AnimationStateMachine::StateVector &AnimationStateMachine::states() const {
    PROFILE_FUNCTION();

    return p_ptr->m_States;
}

AnimationStateMachine::VariableMap &AnimationStateMachine::variables() const {
    PROFILE_FUNCTION();

    return p_ptr->m_Variables;
}
