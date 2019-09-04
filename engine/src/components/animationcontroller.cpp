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

    unordered_map<uint32_t, PropertyAnimation *> m_Properties;

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

    for(auto it : p_ptr->m_Properties) {
        it.second->start();
    }

    setPosition(0);
}

void AnimationController::update() {
    PROFILE_FUNCTION();
    // Check conditions
    for(auto it : p_ptr->m_pCurrentState->m_Transitions) {
        auto variable = p_ptr->m_CurrentVariables.find(it.m_ConditionHash);
        if(variable != p_ptr->m_CurrentVariables.end() && it.checkCondition(variable->second)) {
            crossFade(it.m_pTargetState->m_Hash, 0.75f);
        }
    }

    // Update current clip
    bool nextState = true;
    for(auto it : p_ptr->m_Properties) {
        if(it.second->state() == Animation::RUNNING || it.second->loopCount() == -1) {
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

AnimationStateMachine *AnimationController::stateMachine() const {
    PROFILE_FUNCTION();

    return p_ptr->m_pStateMachine;
}

void AnimationController::setStateMachine(AnimationStateMachine *machine) {
    PROFILE_FUNCTION();

    for(auto it : p_ptr->m_Properties) {
        delete it.second;
    }
    p_ptr->m_Properties.clear();

    p_ptr->m_pStateMachine = machine;
    p_ptr->m_pCurrentState = nullptr;
    if(p_ptr->m_pStateMachine) {
        p_ptr->m_CurrentVariables = p_ptr->m_pStateMachine->variables();
        setState(p_ptr->m_pStateMachine->initialState()->m_Hash);
    }
}

uint32_t AnimationController::position() const {
    PROFILE_FUNCTION();

    return p_ptr->m_Time;
}

void AnimationController::setPosition(uint32_t ms) {
    PROFILE_FUNCTION();

    p_ptr->m_Time = ms;
    for(auto it : p_ptr->m_Properties) {
        it.second->setCurrentTime(p_ptr->m_Time);
    }
}

void AnimationController::setState(const char *state) {
    PROFILE_FUNCTION();

    setState(hash_str(state));
}

void AnimationController::setState(size_t hash) {
    PROFILE_FUNCTION();

    if(p_ptr->m_pStateMachine == nullptr) {
        return;
    }
    if(p_ptr->m_pCurrentState == nullptr || p_ptr->m_pCurrentState->m_Hash != hash) {
        AnimationStateMachine::State *newState = p_ptr->m_pStateMachine->findState(hash);
        if(newState) {
            setClip(newState->m_pClip);
            p_ptr->m_pCurrentState = newState;
            setPosition(0);
        }
    }
}

void AnimationController::crossFade(const char *state, float duration) {
    PROFILE_FUNCTION();

    crossFade(hash_str(state), duration);
}

void AnimationController::crossFade(size_t hash, float duration) {
    PROFILE_FUNCTION();

    if(p_ptr->m_pStateMachine == nullptr) {
        return;
    }
    if(p_ptr->m_pCurrentState == nullptr || p_ptr->m_pCurrentState->m_Hash != hash) {
        AnimationStateMachine::State *newState = p_ptr->m_pStateMachine->findState(hash);
        if(newState) {
            setClips(p_ptr->m_pCurrentState->m_pClip, newState->m_pClip, duration);
            p_ptr->m_pCurrentState = newState;
        }
    }
}

AnimationClip *AnimationController::clip() const {
    PROFILE_FUNCTION();

    return p_ptr->m_pCurrentState->m_pClip;
}

void AnimationController::setClip(AnimationClip *clip) {
    PROFILE_FUNCTION();

    for(auto &it : p_ptr->m_Properties) {
        it.second->setValid(false);
    }

    if(clip == nullptr) {
        return;
    }

    setClips(clip, nullptr);
}

void AnimationController::setClips(AnimationClip *start, AnimationClip *end, float duration, float time) {
    PROFILE_FUNCTION();

    if(end) {
        for(auto &it : p_ptr->m_Properties) {
            it.second->setBeginCurve(nullptr);
            it.second->setEndCurve(nullptr);
        }

        for(auto &it : end->m_Tracks) {
            PropertyAnimation *property;
            auto target = p_ptr->m_Properties.find(it.hash);
            if(target != p_ptr->m_Properties.end()) {
                property = target->second;
            } else {
                property = new PropertyAnimation();
                property->setTarget(findTarget(actor(), it.path), it.property.c_str());

                p_ptr->m_Properties[it.hash] = property;
            }

            property->setValid(true);

            for(auto &i : it.curves) {
                property->setEndCurve(&i.second, i.first);
                property->setOffset(time);
                property->setTransitionTime(duration);
            }
        }
    }

    for(auto &it : start->m_Tracks) {
        PropertyAnimation *property;
        auto target = p_ptr->m_Properties.find(it.hash);
        if(target != p_ptr->m_Properties.end()) {
            property = target->second;
        } else {
            property = new PropertyAnimation();
            property->setTarget(findTarget(actor(), it.path), it.property.c_str());

            p_ptr->m_Properties[it.hash] = property;
        }

        property->setValid(true);

        for(auto &i : it.curves) {
            property->setBeginCurve(&i.second, i.first);
            property->setTransitionTime(duration);
        }
    }
}

void AnimationController::setBool(const char *name, bool value) {
    PROFILE_FUNCTION();

    setBool(hash_str(name), value);
}

void AnimationController::setBool(size_t hash, bool value) {
    PROFILE_FUNCTION();

    auto variable = p_ptr->m_CurrentVariables.find(hash);
    if(variable != p_ptr->m_CurrentVariables.end()) {
        variable->second = value;
    }
}

void AnimationController::setFloat(const char *name, float value) {
    PROFILE_FUNCTION();

    setFloat(hash_str(name), value);
}

void AnimationController::setFloat(size_t hash, float value) {
    PROFILE_FUNCTION();

    auto variable = p_ptr->m_CurrentVariables.find(hash);
    if(variable != p_ptr->m_CurrentVariables.end()) {
        variable->second = value;
    }
}

void AnimationController::setInteger(const char *name, int32_t value) {
    PROFILE_FUNCTION();

    setInteger(hash_str(name), value);
}

void AnimationController::setInteger(size_t hash, int32_t value) {
    PROFILE_FUNCTION();

    auto variable = p_ptr->m_CurrentVariables.find(hash);
    if(variable != p_ptr->m_CurrentVariables.end()) {
        variable->second = value;
    }
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
            size_t index = path.find('/', 1);
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
