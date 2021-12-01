#include "components/animator.h"

#include "components/actor.h"

#include "private/baseanimationblender.h"

#include "resources/animationclip.h"
#include "resources/animationstatemachine.h"

#include "timer.h"

#include "log.h"

#define CLIP "Clip"

static hash<string> hash_str;

class AnimatorPrivate : public Resource::IObserver {
public:
    explicit AnimatorPrivate(Animator *object) :
        d_ptr(object),
        m_pStateMachine(nullptr),
        m_pCurrentState(nullptr),
        m_pCurrentClip(nullptr),
        m_Time(0) {

    }

    void setPosition(uint32_t position, bool force) {
        PROFILE_FUNCTION();

        m_Time = position;
        for(auto it : m_Properties) {
            if(force && it.second->state() == Animation::STOPPED) {
                it.second->start();
            }
            it.second->setCurrentTime(m_Time);
        }
    }

    void setStateHash(int hash) {
        PROFILE_FUNCTION();

        if(m_pStateMachine == nullptr) {
            return;
        }
        if(m_pCurrentState == nullptr || m_pCurrentState->m_hash != hash) {
            AnimationState *newState = m_pStateMachine->findState(hash);
            if(newState) {
                m_pCurrentState = newState;
                setClip(m_pCurrentState->m_clip);
                setPosition(0, false);
            }
#ifdef NEXT_SHARED
            else {
              Log(Log::DBG) << "Unable to make the transition to state" << hash;
            }
#endif
        }
    }

    void resourceUpdated(const Resource *resource, Resource::ResourceState state) override {
        PROFILE_FUNCTION();

        if(resource == m_pStateMachine && state == Resource::Ready) {
            for(auto it : m_Properties) {
                delete it.second;
            }
            m_Properties.clear();

            m_pCurrentState = nullptr;
            if(m_pStateMachine) {
                m_CurrentVariables = m_pStateMachine->variables();
                setStateHash(m_pStateMachine->initialState()->m_hash);
            }
        }
    }

    void setClip(AnimationClip *clip) {
        PROFILE_FUNCTION();

        for(auto &it : m_Properties) {
            it.second->setValid(false);
        }

        if(clip == nullptr) {
            return;
        }

        setClips(nullptr, clip);
    }

    void rebind() {
        PROFILE_FUNCTION();

        if(m_pCurrentClip) {
            Actor *actor = d_ptr->actor();
            for(auto &it : m_pCurrentClip->m_Tracks) {
                BaseAnimationBlender *property = nullptr;
                auto target = m_Properties.find(it.hash());
                if(target != m_Properties.end()) {
                    property = target->second;
                } else {
                property = new BaseAnimationBlender();
                    Object *object = actor->find(it.path());
#ifdef NEXT_SHARED
                    if(object == nullptr) {
                        Log(Log::DBG) << "Can't resolve animation path:" << it.path().c_str();
                    }
#endif
                    property->setTarget(object, it.property().c_str());
                    m_Properties[it.hash()] = property;
                }

                property->setValid(true);
                property->setDuration(it.duration());
                for(auto &i : it.curves()) {
                    property->setCurve(&i.second, i.first);
                }
            }
        }
    }

    void setClips(AnimationClip *start, AnimationClip *end, float duration = 0.0f, float time = 0.0f) {
        PROFILE_FUNCTION();

        Actor *actor = d_ptr->actor();
        if(start) {
            for(auto &it : start->m_Tracks) {
                BaseAnimationBlender *property = nullptr;
                auto target = m_Properties.find(it.hash());
                if(target != m_Properties.end()) {
                    property = target->second;
                    property->setValid(true);
                    property->setLoopCount(1);
                    property->setPreviousDuration(it.duration());
                    for(auto &i : it.curves()) {
                        property->setPreviousCurve(&i.second, i.first);
                        property->setOffset(time);
                        property->setTransitionTime(duration);
                    }
                }
            }
        }

        m_pCurrentClip = end;

        for(auto &it : m_Properties) {
            it.second->stop();
        }

        for(auto &it : end->m_Tracks) {
            BaseAnimationBlender *property;
            auto target = m_Properties.find(it.hash());
            if(target != m_Properties.end()) {
                property = target->second;
            } else {
                property = new BaseAnimationBlender();
                Object *object = actor->find(it.path());
#ifdef NEXT_SHARED
                if(object == nullptr) {
                    Log(Log::DBG) << "Can't resolve animation path:" << it.path().c_str();
                }
#endif
                property->setTarget(object, it.property().c_str());
                m_Properties[it.hash()] = property;
            }
            property->setValid(true);

            if(m_pCurrentState && !m_pCurrentState->m_loop) {
                property->setLoopCount(1);
            } else {
                property->setLoopCount(-1);
            }
            property->setDuration(it.duration());

            for(auto &i : it.curves()) {
                property->setCurve(&i.second, i.first);
                property->setTransitionTime(duration);
            }
            property->start();
        }
    }

    unordered_map<uint32_t, BaseAnimationBlender *> m_Properties;

    AnimationStateMachine::VariableMap m_CurrentVariables;

    Animator *d_ptr;

    AnimationStateMachine *m_pStateMachine;

    AnimationState *m_pCurrentState;

    AnimationClip *m_pCurrentClip;

    uint32_t m_Time;
};

/*!
    \class Animator
    \brief Manages all animations in the engine.
    \inmodule Engine

    The animation controller allows to control different animation states from AnimationStateMachine assets.
    To use any animations in the Thunder Engine the Animator must be attached to a root Actor in the hierarchy.
    The animation controller processes an AnimationStateMachine resource type.
    The animation controller can use parametric values to decide when transition between states must happen.
*/

Animator::Animator() :
        p_ptr(new AnimatorPrivate(this)) {

}

Animator::~Animator() {
    delete p_ptr;
    p_ptr = nullptr;
}
/*!
    \internal
*/
void Animator::start() {
    PROFILE_FUNCTION();

    for(auto it : p_ptr->m_Properties) {
        it.second->start();
    }

    setPosition(0);
}
/*!
    \internal
*/
void Animator::update() {
    PROFILE_FUNCTION();
    if(p_ptr->m_pCurrentState) {
        // Check conditions
        for(auto it : p_ptr->m_pCurrentState->m_transitions) {
            auto variable = p_ptr->m_CurrentVariables.find(it.m_conditionHash);
            if(variable != p_ptr->m_CurrentVariables.end() && it.checkCondition(variable->second)) {
                crossFadeHash(it.m_targetState->m_hash, 0.75f);
            }
        }

        // Update current clip
        bool nextState = true;
        for(auto it : p_ptr->m_Properties) {
            if(it.second->state() == Animation::RUNNING) {
                string str = it.second->targetProperty();
                nextState = false;
                break;
            }
        }

        if(nextState && !p_ptr->m_pCurrentState->m_transitions.empty()) {
            auto next = p_ptr->m_pCurrentState->m_transitions.begin();
            setStateHash(next->m_targetState->m_hash);
        } else {
            setPosition(p_ptr->m_Time + static_cast<uint32_t>(1000.0f * Timer::deltaTime()));
        }
    }
}
/*!
    Returns AnimationStateMachine resource attached to this Animator.
*/
AnimationStateMachine *Animator::stateMachine() const {
    PROFILE_FUNCTION();

    return p_ptr->m_pStateMachine;
}
/*!
    Sets AnimationStateMachine \a resource which will be attached to this Animator.
    \note The state machine will move to the initial state automatically during the call of this function.
*/
void Animator::setStateMachine(AnimationStateMachine *resource) {
    PROFILE_FUNCTION();

    for(auto it : p_ptr->m_Properties) {
        delete it.second;
    }
    p_ptr->m_Properties.clear();

    p_ptr->m_pStateMachine = resource;
    p_ptr->m_pCurrentState = nullptr;
    if(p_ptr->m_pStateMachine) {
        p_ptr->m_CurrentVariables = p_ptr->m_pStateMachine->variables();
        setStateHash(p_ptr->m_pStateMachine->initialState()->m_hash);
    }
}
/*!
    Returns the position (in milliseconds) of animation for the current state.
    \internal
*/
uint32_t Animator::position() const {
    PROFILE_FUNCTION();

    return p_ptr->m_Time;
}
/*!
    Sets the \a position (in milliseconds) of animation for the current state.
    \internal
*/
void Animator::setPosition(uint32_t position) {
    PROFILE_FUNCTION();

    p_ptr->setPosition(position, true);
}
/*!
    Changes the current \a state of state machine immediately.
*/
void Animator::setState(const string &state) {
    PROFILE_FUNCTION();

    setStateHash(hash_str(state));
}
/*!
    Changes the current state (using the \a hash of state) of state machine immediately.
*/
void Animator::setStateHash(int hash) {
    PROFILE_FUNCTION();

    p_ptr->setStateHash(hash);
}
/*!
    Smoothly changes current state using crossfade interpolation from the previous state to the new \a state with \a duration (in milliseconds).
*/
void Animator::crossFade(const string &state, float duration) {
    PROFILE_FUNCTION();

    crossFadeHash(hash_str(state), duration);
}
/*!
    Smoothly changes current state using crossfade interpolation from the previous state to the new state (using the \a hash of state) with \a duration (in milliseconds).
*/
void Animator::crossFadeHash(int hash, float duration) {
    PROFILE_FUNCTION();

    if(p_ptr->m_pStateMachine == nullptr) {
        return;
    }
    if(p_ptr->m_pCurrentState == nullptr || p_ptr->m_pCurrentState->m_hash != hash) {
        AnimationState *newState = p_ptr->m_pStateMachine->findState(hash);
        if(newState) {
            if(p_ptr->m_pCurrentState) {
                p_ptr->setClips(p_ptr->m_pCurrentState->m_clip, newState->m_clip, duration);
            }
            p_ptr->m_pCurrentState = newState;
        }
    }
}
/*!
    Forcefully sets animation \a clip over any state.
    \internal
*/
void Animator::setClip(AnimationClip *clip) {
    PROFILE_FUNCTION();

    p_ptr->setClip(clip);
}

/*!
    Rebinds all animated properties with Animator.
*/
void Animator::rebind() {
    PROFILE_FUNCTION();

    p_ptr->rebind();
}
/*!
    Sets the new boolean \a value for the parameter with the \a name.
*/
void Animator::setBool(const string &name, bool value) {
    PROFILE_FUNCTION();

    setBoolHash(hash_str(name), value);
}
/*!
    Sets the new boolean \a value for the parameter using the \a hash of state as the name.
*/
void Animator::setBoolHash(int hash, bool value) {
    PROFILE_FUNCTION();

    auto variable = p_ptr->m_CurrentVariables.find(hash);
    if(variable != p_ptr->m_CurrentVariables.end()) {
        variable->second = value;
    }
}
/*!
    Sets the new floating-point \a value for the parameter with the \a name.
*/
void Animator::setFloat(const string &name, float value) {
    PROFILE_FUNCTION();

    setFloatHash(hash_str(name), value);
}
/*!
    Sets the new floating-point \a value for the parameter using the \a hash of state as the name.
*/
void Animator::setFloatHash(int hash, float value) {
    PROFILE_FUNCTION();

    auto variable = p_ptr->m_CurrentVariables.find(hash);
    if(variable != p_ptr->m_CurrentVariables.end()) {
        variable->second = value;
    }
}
/*!
    Sets the new integer \a value for the parameter with the \a name.
*/
void Animator::setInteger(const string &name, int32_t value) {
    PROFILE_FUNCTION();

    setIntegerHash(hash_str(name), value);
}
/*!
    Sets the new integer \a value for the parameter using the \a hash of state as the name.
*/
void Animator::setIntegerHash(int hash, int32_t value) {
    PROFILE_FUNCTION();

    auto variable = p_ptr->m_CurrentVariables.find(hash);
    if(variable != p_ptr->m_CurrentVariables.end()) {
        variable->second = value;
    }
}
/*!
    Returns duration of the animation clip for the current state.
*/
int Animator::duration() const {
    PROFILE_FUNCTION();
    if(p_ptr->m_pCurrentState) {
        return p_ptr->m_pCurrentState->m_clip->duration();
    }
    return 0;
}
/*!
    \internal
*/
void Animator::loadUserData(const VariantMap &data) {
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
VariantMap Animator::saveUserData() const {
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
