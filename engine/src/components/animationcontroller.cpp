#include "components/animationcontroller.h"

#include "components/actor.h"

#include "resources/animationclip.h"
#include "resources/animationstatemachine.h"

#include "timer.h"
#include "anim/propertyanimation.h"

#define CLIP "Clip"

static hash<string> hash_str;

class AnimationControllerPrivate {
public:
    AnimationControllerPrivate() :
        m_pStateMachine(nullptr),
        m_pClip(nullptr),
        m_pCurrentState(nullptr),
        m_Time(0) {

    }

    typedef list<PropertyAnimation *> PropertyList;

    map<AnimationClip *, PropertyList> m_Properties;

    AnimationStateMachine *m_pStateMachine;

    AnimationClip *m_pClip;

    AnimationStateMachine::State *m_pCurrentState;

    AnimationStateMachine::VariableMap m_CurrentVariables;

    uint32_t m_Time;
};

AnimationController::AnimationController() :
        p_ptr(new AnimationControllerPrivate()) {

}

AnimationController::~AnimationController() {
    delete p_ptr;
}

void AnimationController::start() {
    PROFILE_FUNCTION();

    for(auto it : p_ptr->m_Properties[p_ptr->m_pClip]) {
        it->start();
    }

    setPosition(0);
}

void AnimationController::update() {
    PROFILE_FUNCTION();
    // Check conditions
    for(auto it : p_ptr->m_pCurrentState->m_Transitions) {
        auto variable = p_ptr->m_CurrentVariables.find(it.m_ConditionHash);
        if(variable != p_ptr->m_CurrentVariables.end() &&
           variable->second.toBool()) {

            setState(it.m_pTargetState->m_Hash);
            start();
            return;
        }
    }
    // Update current clip
    if(p_ptr->m_pClip) {
        auto list = p_ptr->m_Properties.find(p_ptr->m_pClip);
        if(list != p_ptr->m_Properties.end()) {
            bool nextState = true;
            for(auto it : list->second) {
                if(it->state() == Animation::RUNNING || it->loopCount() == -1) {
                    nextState = false;
                    break;
                }
            }

            if(nextState && !p_ptr->m_pCurrentState->m_Transitions.empty()) {
                auto next = p_ptr->m_pCurrentState->m_Transitions.begin();
                setState(next->m_pTargetState->m_Hash);
            } else {
                setPosition(p_ptr->m_Time + static_cast<uint32_t>(1000.0f * Timer::deltaTime()));
            }
        }
    }

}

AnimationStateMachine *AnimationController::stateMachine() const {
    PROFILE_FUNCTION();

    return p_ptr->m_pStateMachine;
}

void AnimationController::setStateMachine(AnimationStateMachine *machine) {
    PROFILE_FUNCTION();

    for(auto it : p_ptr->m_Properties) {
        for(auto property : it.second) {
            delete property;
        }
    }
    p_ptr->m_Properties.clear();

    p_ptr->m_pStateMachine = machine;
    p_ptr->m_pCurrentState = nullptr;
    if(p_ptr->m_pStateMachine) {
        p_ptr->m_pCurrentState = p_ptr->m_pStateMachine->m_pInitialState;
        for(auto state : p_ptr->m_pStateMachine->m_States) {
            AnimationControllerPrivate::PropertyList list;
            auto target = p_ptr->m_Properties.find(state->m_pClip);
            if(target == p_ptr->m_Properties.end()) {
                for(auto &it : state->m_pClip->m_Tracks) {
                    Object *target = findTarget(actor(), it.path);

                    PropertyAnimation *property = new PropertyAnimation();
                    property->setTarget(target, it.property.c_str());
                    list.push_back(property);

                    for(auto i : it.curves) {
                        property->setCurve(i.second, i.first);
                    }
                }
                p_ptr->m_Properties[state->m_pClip] = list;
            }
        }
    }
}

uint32_t AnimationController::position() const {
    PROFILE_FUNCTION();

    return p_ptr->m_Time;
}

void AnimationController::setPosition(uint32_t ms) {
    PROFILE_FUNCTION();

    p_ptr->m_Time  = ms;
    if(p_ptr->m_pClip) {
        auto list = p_ptr->m_Properties.find(p_ptr->m_pClip);
        if(list != p_ptr->m_Properties.end()) {
            for(auto it : list->second) {
                it->setCurrentTime(p_ptr->m_Time);
            }
        }
    }
}

void AnimationController::setState(string &state) {
    PROFILE_FUNCTION();

    setState(hash_str(state));
}

void AnimationController::setState(size_t hash) {
    PROFILE_FUNCTION();

    AnimationStateMachine::State *newState = p_ptr->m_pStateMachine->findState(hash);
    if(newState) {
        p_ptr->m_pCurrentState = newState;
        p_ptr->m_pClip = p_ptr->m_pCurrentState->m_pClip;
    }
}

void AnimationController::setClip(AnimationClip *clip) {
    p_ptr->m_pClip = clip;
}

uint32_t AnimationController::duration() const {
    PROFILE_FUNCTION();

    return p_ptr->m_pCurrentState->m_pClip->duration();
}

void AnimationController::loadUserData(const VariantMap &data) {
    PROFILE_FUNCTION();

    Component::loadUserData(data);
    {
        auto it = data.find(CLIP);
        if(it != data.end()) {
            setStateMachine(Engine::loadResource<AnimationStateMachine>((*it).second.toString()));
        }
    }
}

VariantMap AnimationController::saveUserData() const {
    PROFILE_FUNCTION();

    VariantMap result = Component::saveUserData();
    {
        string ref = Engine::reference(p_ptr->m_pStateMachine);
        if(!ref.empty()) {
            result[CLIP] = ref;
        }
    }
    return result;
}

Object *AnimationController::findTarget(Object *src, const string &path) {
    PROFILE_FUNCTION();

    if(src) {
        if(path.empty() || path == src->name()) {
            return src;
        } else {
            string sub = path;
            uint32_t index = path.find('/', 1);
            if(index != string::npos) {
                sub = path.substr(index + 1);
            }
            for(const auto &it : src->getChildren()) {
                Object *o  = findTarget(it, sub);
                if(o) {
                    return o;
                }
            }
        }
    }
    return nullptr;
}
