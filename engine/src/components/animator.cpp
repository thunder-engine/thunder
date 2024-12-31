#include "components/animator.h"

#include "components/actor.h"

#include "private/baseanimationblender.h"

#include "resources/animationclip.h"
#include "resources/animationstatemachine.h"

#include "timer.h"

#include "log.h"

/*!
    \class Animator
    \brief Manages all animations in the engine.
    \inmodule Components

    The animation controller allows to control different animation states from AnimationStateMachine assets.
    To use any animations in the Thunder Engine the Animator must be attached to a root Actor in the hierarchy.
    The animation controller processes an AnimationStateMachine resource type.
    The animation controller can use parametric values to decide when transition between states must happen.
*/

Animator::Animator() :
        m_stateMachine(nullptr),
        m_currentState(nullptr),
        m_currentClip(nullptr),
        m_time(0) {

}

Animator::~Animator() {
    if(m_stateMachine) {
        m_stateMachine->unsubscribe(this);
    }
}
/*!
    \internal
*/
void Animator::start() {
    PROFILE_FUNCTION();

    for(auto it : m_properties) {
        it.second->start();
    }

    setPosition(0);
}
/*!
    \internal
*/
void Animator::resume() {
    for(auto it : m_properties) {
        if(it.second->state() == Animation::STOPPED) {
            it.second->resume(true);
        }
    }
}

/*!
    \internal
*/
void Animator::update() {
    PROFILE_FUNCTION();
    if(m_currentState) {
        // Check conditions
        for(auto it : m_currentState->m_transitions) {
            auto variable = m_currentVariables.find(it.m_conditionHash);
            if(variable != m_currentVariables.end() && it.checkCondition(variable->second)) {
                crossFadeHash(it.m_targetState->m_hash, 0.75f);
            }
        }

        // Update current clip
        bool nextState = true;
        for(auto it : m_properties) {
            if(it.second->state() == Animation::RUNNING) {
                //string str = it.second->targetProperty();
                nextState = false;
                break;
            }
        }

        if(nextState && !m_currentState->m_transitions.empty()) {
            auto next = m_currentState->m_transitions.begin();
            setStateHash(next->m_targetState->m_hash);
        } else {
            setPosition(m_time + static_cast<uint32_t>(1000.0f * Timer::deltaTime()));
        }
    }
}
/*!
    Returns AnimationStateMachine resource attached to this Animator.
*/
AnimationStateMachine *Animator::stateMachine() const {
    PROFILE_FUNCTION();

    return m_stateMachine;
}
/*!
    Sets animation state \a machine which will be attached to this Animator.
    \note The state machine will move to the initial state automatically during the call of this function.
*/
void Animator::setStateMachine(AnimationStateMachine *machine) {
    PROFILE_FUNCTION();

    if(m_stateMachine != machine) {
        for(auto it : m_properties) {
            delete it.second;
        }
        m_properties.clear();

        if(m_stateMachine) {
            m_stateMachine->unsubscribe(this);
        }

        m_stateMachine = machine;
        m_currentState = nullptr;
        if(m_stateMachine) {
            m_currentVariables = m_stateMachine->variables();
            AnimationState *initialState = m_stateMachine->initialState();
            if(initialState) {
                setStateHash(initialState->m_hash);
            }

            m_stateMachine->subscribe(&Animator::stateMachineUpdated, this);
        }
    }
}
/*!
    Returns the position (in milliseconds) of animation for the current state.
    \internal
*/
uint32_t Animator::position() const {
    PROFILE_FUNCTION();

    return m_time;
}
/*!
    Sets the \a position (in milliseconds) of animation for the current state.
    \internal
*/
void Animator::setPosition(uint32_t position) {
    PROFILE_FUNCTION();

    m_time = position;

    for(auto it : m_properties) {
        it.second->setCurrentTime(m_time);
    }
}
/*!
    Changes the current \a state of state machine immediately.
*/
void Animator::setState(const std::string &state) {
    PROFILE_FUNCTION();

    setStateHash(Mathf::hashString(state));
}
/*!
    Changes the current state (using the \a hash of state) of state machine immediately.
*/
void Animator::setStateHash(int hash) {
    PROFILE_FUNCTION();

    PROFILE_FUNCTION();

    if(m_stateMachine == nullptr) {
        return;
    }
    if(m_currentState == nullptr || m_currentState->m_hash != hash) {
        AnimationState *newState = m_stateMachine->findState(hash);
        if(newState) {
            m_currentState = newState;
            setClip(m_currentState->m_clip);
            m_time = 0;
            for(auto it : m_properties) {
                it.second->setCurrentTime(m_time);
            }
        }
#ifdef SHARED_DEFINE
        else {
            aDebug() << "Unable to make the transition to state" << hash;
        }
#endif
    }
}
/*!
    Smoothly changes current state using crossfade interpolation from the previous state to the new \a state with \a duration (in milliseconds).
*/
void Animator::crossFade(const std::string &state, float duration) {
    PROFILE_FUNCTION();

    crossFadeHash(Mathf::hashString(state), duration);
}
/*!
    Smoothly changes current state using crossfade interpolation from the previous state to the new state (using the \a hash of state) with \a duration (in milliseconds).
*/
void Animator::crossFadeHash(int hash, float duration) {
    PROFILE_FUNCTION();

    if(m_stateMachine == nullptr) {
        return;
    }
    if(m_currentState == nullptr || m_currentState->m_hash != hash) {
        AnimationState *newState = m_stateMachine->findState(hash);
        if(newState) {
            if(m_currentState) {
                setClips(m_currentState->m_clip, newState->m_clip, duration);
            }
            m_currentState = newState;
        }
    }
}
/*!
    Forcefully sets animation \a clip over any state.
    \internal
*/
void Animator::setClip(AnimationClip *clip) {
    PROFILE_FUNCTION();

    for(auto &it : m_properties) {
        it.second->setValid(false);
    }

    if(clip == nullptr) {
        return;
    }

    setClips(nullptr, clip);
}

/*!
    Rebinds all animated properties with Animator.
*/
void Animator::rebind() {
    PROFILE_FUNCTION();

    if(m_currentClip) {
        Actor *a = actor();
        for(auto &it : m_currentClip->m_tracks) {
            BaseAnimationBlender *property = nullptr;
            auto target = m_properties.find(it.hash());
            if(target != m_properties.end()) {
                property = target->second;
            } else {
                property = new BaseAnimationBlender();
                Object *object = a->find(it.path());
#ifdef SHARED_DEFINE
                if(object == nullptr) {
                    aDebug() << "Can't resolve animation path:" << it.path();
                }
#endif
                property->setTarget(object, it.property().c_str());
                m_properties[it.hash()] = property;
            }

            property->setValid(true);
            property->setDuration(it.duration());
            property->setCurve(it.curve());
        }
    }
}
/*!
    Sets the new boolean \a value for the parameter with the \a name.
*/
void Animator::setBool(const std::string &name, bool value) {
    PROFILE_FUNCTION();

    setBoolHash(Mathf::hashString(name), value);
}
/*!
    Sets the new boolean \a value for the parameter using the \a hash of state as the name.
*/
void Animator::setBoolHash(int hash, bool value) {
    PROFILE_FUNCTION();

    auto variable = m_currentVariables.find(hash);
    if(variable != m_currentVariables.end()) {
        variable->second = value;
    }
}
/*!
    Sets the new floating-point \a value for the parameter with the \a name.
*/
void Animator::setFloat(const std::string &name, float value) {
    PROFILE_FUNCTION();

    setFloatHash(Mathf::hashString(name), value);
}
/*!
    Sets the new floating-point \a value for the parameter using the \a hash of state as the name.
*/
void Animator::setFloatHash(int hash, float value) {
    PROFILE_FUNCTION();

    auto variable = m_currentVariables.find(hash);
    if(variable != m_currentVariables.end()) {
        variable->second = value;
    }
}
/*!
    Sets the new integer \a value for the parameter with the \a name.
*/
void Animator::setInteger(const std::string &name, int32_t value) {
    PROFILE_FUNCTION();

    setIntegerHash(Mathf::hashString(name), value);
}
/*!
    Sets the new integer \a value for the parameter using the \a hash of state as the name.
*/
void Animator::setIntegerHash(int hash, int32_t value) {
    PROFILE_FUNCTION();

    auto variable = m_currentVariables.find(hash);
    if(variable != m_currentVariables.end()) {
        variable->second = value;
    }
}
/*!
    Returns duration of the animation clip for the current state.
*/
int Animator::duration() const {
    PROFILE_FUNCTION();
    if(m_currentState) {
        return m_currentState->m_clip->duration();
    }
    return 0;
}
/*!
    \internal
*/
void Animator::setClips(AnimationClip *start, AnimationClip *end, float duration, float time) {
    PROFILE_FUNCTION();

    if(start) {
        for(auto &it : start->m_tracks) {
            BaseAnimationBlender *property = nullptr;
            auto target = m_properties.find(it.hash());
            if(target != m_properties.end()) {
                property = target->second;
                property->setValid(true);
                property->setLoopCount(1);
                property->setPreviousDuration(it.duration());
                property->setPreviousTrack(it);
                property->setOffset(time);
                property->setTransitionTime(duration);
            }
        }
    }

    m_currentClip = end;

    for(auto &it : m_properties) {
        it.second->stop();
    }

    Actor *a = actor();

    for(auto &it : end->m_tracks) {
        BaseAnimationBlender *property;
        auto target = m_properties.find(it.hash());
        if(target != m_properties.end()) {
            property = target->second;
        } else {
            property = new BaseAnimationBlender();
            Object *object = a->find(it.path());
#ifdef SHARED_DEFINE
            if(object == nullptr) {
                aDebug() << "Can't resolve animation path:" << it.path();
            }
#endif
            property->setTarget(object, it.property().c_str());
            m_properties[it.hash()] = property;
        }
        property->setValid(true);

        if(m_currentState && !m_currentState->m_loop) {
            property->setLoopCount(1);
        } else {
            property->setLoopCount(-1);
        }

        property->setDuration(it.duration());
        property->setCurve(it.curve());
        property->setTransitionTime(duration);

        property->start();
    }
}
/*!
    \internal
*/
void Animator::stateMachineUpdated(int state, void *ptr) {
    PROFILE_FUNCTION();

    if(state == Resource::Ready) {
        Animator *p = static_cast<Animator *>(ptr);

        for(auto it : p->m_properties) {
            delete it.second;
        }
        p->m_properties.clear();

        p->m_currentState = nullptr;
        if(p->m_stateMachine) {
            p->m_currentVariables = p->m_stateMachine->variables();
            AnimationState *initialState = p->m_stateMachine->initialState();
            if(initialState) {
                p->setStateHash(initialState->m_hash);
            }
        }
    }
}
