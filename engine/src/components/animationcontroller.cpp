#include "components/animationcontroller.h"

#include "components/actor.h"

#include "resources/animationclip.h"
#include "resources/animationstatemachine.h"

#include "timer.h"
#include "anim/propertyanimation.h"

#include "log.h"

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

/*!
    \class AnimationController
    \brief Manages all animations in the engine.
    \inmodule Engine

    The animation controller allows to control different animation states from AnimationStateMachine assets.
    To use any animations in the Thunder Engine the AnimationController must be attached to a root Actor in the hierarchy.
    The animation controller processes an AnimationStateMachine resource type.
    The animation controller can use parametric values to decide when transition between states must happen.
*/

AnimationController::AnimationController() :
        p_ptr(new AnimationControllerPrivate()) {

}

AnimationController::~AnimationController() {
    delete p_ptr;
}
/*!
    \internal
*/
void AnimationController::start() {
    PROFILE_FUNCTION();

    for(auto it : p_ptr->m_Properties) {
        it.second->start();
    }

    setPosition(0);
}
/*!
    \internal
*/
void AnimationController::update() {
    PROFILE_FUNCTION();
    // Check conditions
    for(auto it : p_ptr->m_pCurrentState->m_Transitions) {
        auto variable = p_ptr->m_CurrentVariables.find(it.m_ConditionHash);
        if(variable != p_ptr->m_CurrentVariables.end() && it.checkCondition(variable->second)) {
            crossFadeHash(it.m_pTargetState->m_Hash, 0.75f);
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
        setStateHash(next->m_pTargetState->m_Hash);
    } else {
        setPosition(p_ptr->m_Time + static_cast<uint32_t>(1000.0f * Timer::deltaTime()));
    }
}
/*!
    Returns AnimationStateMachine resource attached to this AnimationController.
*/
AnimationStateMachine *AnimationController::stateMachine() const {
    PROFILE_FUNCTION();

    return p_ptr->m_pStateMachine;
}
/*!
    Sets AnimationStateMachine \a resource which will be attached to this AnimationController.
    \note The state machine will move to the initial state automatically during the call of this function.
*/
void AnimationController::setStateMachine(AnimationStateMachine *resource) {
    PROFILE_FUNCTION();

    for(auto it : p_ptr->m_Properties) {
        delete it.second;
    }
    p_ptr->m_Properties.clear();

    p_ptr->m_pStateMachine = resource;
    p_ptr->m_pCurrentState = nullptr;
    if(p_ptr->m_pStateMachine) {
        p_ptr->m_CurrentVariables = p_ptr->m_pStateMachine->variables();
        setStateHash(p_ptr->m_pStateMachine->initialState()->m_Hash);
    }
}
/*!
    Returns the position (in milliseconds) of animation for the current state.
    \internal
*/
uint32_t AnimationController::position() const {
    PROFILE_FUNCTION();

    return p_ptr->m_Time;
}
/*!
    Sets the \a position (in milliseconds) of animation for the current state.
    \internal
*/
void AnimationController::setPosition(uint32_t position) {
    PROFILE_FUNCTION();

    p_ptr->m_Time = position;
    for(auto it : p_ptr->m_Properties) {
        it.second->setCurrentTime(p_ptr->m_Time);
    }
}
/*!
    Changes the current \a state of state machine immediately.
*/
void AnimationController::setState(const string &state) {
    PROFILE_FUNCTION();

    setStateHash(hash_str(state));
}
/*!
    Changes the current state (using the \a hash of state) of state machine immediately.
*/
void AnimationController::setStateHash(int hash) {
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
/*!
    Smoothly changes current state using crossfade interpolation from the previous state to the new \a state with \a duration (in milliseconds).
*/
void AnimationController::crossFade(string &state, float duration) {
    PROFILE_FUNCTION();

    crossFadeHash(hash_str(state), duration);
}
/*!
    Smoothly changes current state using crossfade interpolation from the previous state to the new state (using the \a hash of state) with \a duration (in milliseconds).
*/
void AnimationController::crossFadeHash(int hash, float duration) {
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
/*!
    Returns AnimationClip for the current state.
*/
AnimationClip *AnimationController::clip() const {
    PROFILE_FUNCTION();

    return p_ptr->m_pCurrentState->m_pClip;
}
/*!
    Forcefully sets animation \a clip over any state.
*/
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
/*!
    \internal
*/
void AnimationController::setClips(AnimationClip *start, AnimationClip *end, float duration, float time) {
    PROFILE_FUNCTION();

    if(end) {
        for(auto &it : p_ptr->m_Properties) {
            it.second->setBeginCurve(nullptr);
            it.second->setEndCurve(nullptr);
        }

        for(auto &it : end->m_Tracks) {
            PropertyAnimation *property;
            auto target = p_ptr->m_Properties.find(it.hash());
            if(target != p_ptr->m_Properties.end()) {
                property = target->second;
            } else {
                property = new PropertyAnimation();
                property->setTarget(findTarget(actor(), it.path()), it.property().c_str());

                p_ptr->m_Properties[it.hash()] = property;
            }

            property->setValid(true);

            for(auto &i : it.curves()) {
                property->setEndCurve(&i.second, i.first);
                property->setOffset(time);
                property->setTransitionTime(duration);
            }
        }
    }

    for(auto &it : start->m_Tracks) {
        PropertyAnimation *property;
        auto target = p_ptr->m_Properties.find(it.hash());
        if(target != p_ptr->m_Properties.end()) {
            property = target->second;
        } else {
            property = new PropertyAnimation();
            Object *object = actor()->find(it.path());
#ifdef NEXT_SHARED
            if(object == nullptr) {
                Log(Log::DBG) << "Can't resolve animation path:" << it.path().c_str();
            }
#endif
            property->setTarget(object, it.property().c_str());

            p_ptr->m_Properties[it.hash()] = property;
        }

        property->setValid(true);

        for(auto &i : it.curves()) {
            property->setBeginCurve(&i.second, i.first);
            property->setTransitionTime(duration);
        }
    }
}
/*!
    Sets the new boolean \a value for the parameter with the \a name.
*/
void AnimationController::setBool(const string &name, bool value) {
    PROFILE_FUNCTION();

    setBoolHash(hash_str(name), value);
}
/*!
    Sets the new boolean \a value for the parameter using the \a hash of state as the name.
*/
void AnimationController::setBoolHash(int hash, bool value) {
    PROFILE_FUNCTION();

    auto variable = p_ptr->m_CurrentVariables.find(hash);
    if(variable != p_ptr->m_CurrentVariables.end()) {
        variable->second = value;
    }
}
/*!
    Sets the new floating-point \a value for the parameter with the \a name.
*/
void AnimationController::setFloat(const string &name, float value) {
    PROFILE_FUNCTION();

    setFloatHash(hash_str(name), value);
}
/*!
    Sets the new floating-point \a value for the parameter using the \a hash of state as the name.
*/
void AnimationController::setFloatHash(int hash, float value) {
    PROFILE_FUNCTION();

    auto variable = p_ptr->m_CurrentVariables.find(hash);
    if(variable != p_ptr->m_CurrentVariables.end()) {
        variable->second = value;
    }
}
/*!
    Sets the new integer \a value for the parameter with the \a name.
*/
void AnimationController::setInteger(const string &name, int32_t value) {
    PROFILE_FUNCTION();

    setIntegerHash(hash_str(name), value);
}
/*!
    Sets the new integer \a value for the parameter using the \a hash of state as the name.
*/
void AnimationController::setIntegerHash(int hash, int32_t value) {
    PROFILE_FUNCTION();

    auto variable = p_ptr->m_CurrentVariables.find(hash);
    if(variable != p_ptr->m_CurrentVariables.end()) {
        variable->second = value;
    }
}
/*!
    Returns duration of the animation clip for the current state.
*/
int AnimationController::duration() const {
    PROFILE_FUNCTION();

    return p_ptr->m_pCurrentState->m_pClip->duration();
}
/*!
    \internal
*/
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
/*!
    \internal
*/
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
/*!
    \internal
*/
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
