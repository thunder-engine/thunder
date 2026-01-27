#include "components/animator.h"

#include "components/actor.h"

#include "resources/animationclip.h"
#include "resources/animationstatemachine.h"

#include "timer.h"
#include <log.h>

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
        m_transitionDuration(0.0f) {

}

Animator::~Animator() {
    if(m_stateMachine) {
        m_stateMachine->unsubscribe(this);
    }
}
/*!
    \internal
*/
void Animator::update() {
    PROFILE_FUNCTION();

    checkNextState();

    process(1000.0f * Timer::deltaTime());
}
/*!
    \internal
*/
void Animator::process(float dt) {
    PROFILE_FUNCTION();

    for(auto &it : m_bindProperties) {
        TargetProperties &target = it.second;

        switch(target.defaultValue.type()) {
            case MetaType::QUATERNION: sampleQuaternion(dt, target); break;
            case MetaType::STRING: sampleString(dt, target); break;
            default: sampleVector4(dt, target); break;
        }
    }

    if(m_transitionDuration > 0.0f) {
        checkEndOfTransition();
    }
}

void Animator::sampleVector4(float dt, TargetProperties &target) {
    Vector4 vec4;

    float factor = 0.0f;

    auto playbackIt = target.playbacks.begin();
    while(playbackIt != target.playbacks.end()) {
        PlaybackState &playback = *playbackIt;
        float position = playback.currentPosition + dt / playback.motion->duration();
        bool endOfPlayback = recalcTransitionWeights(playback, position, factor);

        if(playback.weight > 0.0f) {
            if(updatePosition(playback, position)) {
                endOfPlayback = true;
            }

            vec4 += playback.motion->valueVector4(playback.currentPosition) * playback.weight;
        }

        if(endOfPlayback) {
            playbackIt = target.playbacks.erase(playbackIt);
        } else {
            ++playbackIt;
        }
    }

    switch(target.defaultValue.type()) {
        case MetaType::BOOLEAN: target.property.write(target.object, (bool)vec4.x); break;
        case MetaType::INTEGER: target.property.write(target.object, (int)vec4.x); break;
        case MetaType::FLOAT: target.property.write(target.object, vec4.x); break;
        case MetaType::VECTOR2: target.property.write(target.object, Vector2(vec4)); break;
        case MetaType::VECTOR3: target.property.write(target.object, Vector3(vec4)); break;
        default: target.property.write(target.object, vec4); break;
    }
}

void Animator::sampleQuaternion(float dt, TargetProperties &target) {
    Quaternion quat;

    float factor = 0.0f;

    auto playbackIt = target.playbacks.begin();
    while(playbackIt != target.playbacks.end()) {
        PlaybackState &playback = *playbackIt;
        float position = playback.currentPosition + dt / playback.motion->duration();
        bool endOfPlayback = recalcTransitionWeights(playback, position, factor);

        if(playback.weight > 0.0f) {
            if(updatePosition(playback, position)) {
                endOfPlayback = true;
            }

            quat.mix(quat, playback.motion->valueQuaternion(playback.currentPosition), playback.weight);
        }

        if(endOfPlayback) {
            playbackIt = target.playbacks.erase(playbackIt);
        } else {
            ++playbackIt;
        }
    }

    target.property.write(target.object, quat);
}

void Animator::sampleString(float dt, TargetProperties &target) {
    TString str;

    float factor = 0.0f;

    auto playbackIt = target.playbacks.begin();
    while(playbackIt != target.playbacks.end()) {
        PlaybackState &playback = *playbackIt;
        float position = playback.currentPosition + dt / playback.motion->duration();
        bool endOfPlayback = recalcTransitionWeights(playback, position, factor);

        if(playback.weight > 0.0f) {
            if(updatePosition(playback, position)) {
                endOfPlayback = true;
            }

            str = playback.motion->valueString(playback.currentPosition);
        }

        if(endOfPlayback) {
            playbackIt = target.playbacks.erase(playbackIt);
        } else {
            ++playbackIt;
        }
    }

    target.property.write(target.object, str);
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
        m_bindProperties.clear();

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
    Changes the current \a state of state machine immediately.
*/
void Animator::setState(const TString &state) {
    PROFILE_FUNCTION();

    setStateHash(Mathf::hashString(state));
}
/*!
    Changes the current state (using the \a hash of state) of state machine immediately.
*/
void Animator::setStateHash(int hash) {
    PROFILE_FUNCTION();

    crossFadeHash(hash, 0.0f);
}
/*!
    Smoothly changes current state using crossfade interpolation from the previous state to the new \a state with normalized \a duration time.
*/
void Animator::crossFade(const TString &state, float duration) {
    PROFILE_FUNCTION();

    crossFadeHash(Mathf::hashString(state), duration);
}
/*!
    Smoothly changes current state using crossfade interpolation from the previous state to the new state (using the \a hash of state) with normalized \a duration time.
*/
void Animator::crossFadeHash(int hash, float duration) {
    PROFILE_FUNCTION();

    if(m_stateMachine == nullptr) {
        return;
    }
    if(m_currentState == nullptr || m_currentState->m_hash != hash) {
        AnimationState *newState = m_stateMachine->findState(hash);
        if(newState) {
            m_currentState = newState;
            m_transitionDuration = duration;

            setClip(m_currentState->m_clip);
        }
    }
}
/*!
    Forcefully sets animation \a clip over any state.
    Optional normalized \a position argument can be used to set the exact place of clip to play.
    \internal
*/
void Animator::setClip(AnimationClip *clip, float position) {
    PROFILE_FUNCTION();

    if(m_transitionDuration == 0.0f) {
        for(auto &it : m_bindProperties) {
            it.second.playbacks.clear();
        }
    }

    if(clip == nullptr) {
        return;
    }

    m_currentClip = clip;

    rebind();

    if(position > 0.0f) {
        process(MIN(position, 1.0f) * m_currentClip->duration());
    }
}
/*!
    Rebinds all animated properties with Animator.
*/
void Animator::rebind() {
    PROFILE_FUNCTION();

    if(m_currentClip) {
        Actor *actor = Animator::actor();
        for(auto &it : m_currentClip->tracks()) {
            TargetProperties *target = nullptr;

            auto bind = m_bindProperties.find(it.hash());
            if(bind != m_bindProperties.end()) {
                target = &(bind->second);
            } else {
                Object *object = actor->find(it.path());
                if(object) {
                    const MetaObject *meta = object->metaObject();
                    int32_t index = meta->indexOfProperty(it.property().data());
                    if(index > -1) {
                        TargetProperties data;
                        data.property = meta->property(index);
                        data.object = object;
                        data.defaultValue = data.property.read(data.object);

                        m_bindProperties[it.hash()] = data;
                        target = &m_bindProperties[it.hash()];
                    }
                }
#ifdef SHARED_DEFINE
                else {
                    aDebug() << "Can't resolve animation path:" << it.path();
                }
#endif
            }

            if(target) {
                float weight = (m_transitionDuration != 0.0f) ? 0.0f : 1.0f;
                target->playbacks.push_back({ &it, m_currentState, 0.0f, weight });
            }
        }
    }
}
/*!
    Sets the new boolean \a value for the parameter with the \a name.
*/
void Animator::setBool(const TString &name, bool value) {
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
void Animator::setFloat(const TString &name, float value) {
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
void Animator::setInteger(const TString &name, int32_t value) {
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
    \internal
*/
void Animator::stateMachineUpdated(int state, void *ptr) {
    PROFILE_FUNCTION();

    if(state == Resource::Ready) {
        Animator *p = static_cast<Animator *>(ptr);

        p->m_bindProperties.clear();
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
/*!
    \internal
*/
void Animator::checkNextState() {
    PROFILE_FUNCTION();

    if(m_currentState) {
        bool nextState = !m_currentState->m_loop;
        if(nextState) {
            for(auto &it : m_bindProperties) {
                if(!it.second.playbacks.empty()) {
                    nextState = false;
                    break;
                }
            }
        }

        for(auto &it : m_currentState->m_transitions) {
            if(!nextState && !it.m_conditions.empty()) {
                nextState = true;
                for(auto &condition : it.m_conditions) { // We checking all conditions rules because of logical AND rule
                    auto variable = m_currentVariables.find(condition.m_hash);
                    if(variable == m_currentVariables.end() || !condition.check(variable->second)) {
                        nextState = false;
                        break;
                    }
                }
            }

            if(nextState) {
                crossFadeHash(it.m_targetState->m_hash, it.m_duration);
                break;
            }
        }
    }
}
/*!
    \internal
*/
void Animator::checkEndOfTransition() {
    bool endOfTransition = true;

    for(auto &it : m_bindProperties) {
        const TargetProperties &target = it.second;
        for(const auto &playback : target.playbacks) {
            if(playback.state != m_currentState) {
                endOfTransition = false;
                break;
            }
        }
    }

    if(endOfTransition) {
        m_transitionDuration = 0.0f;
    }
}
/*!
    \internal
*/
bool Animator::recalcTransitionWeights(PlaybackState &playback, float position, float &factor) const {
    if(m_transitionDuration > 0.0f) {
        float offset = 1.0f - m_transitionDuration;

        if(playback.state != m_currentState) {
            factor = MAX((position - offset) / m_transitionDuration, 0.0f);
            playback.weight = 1.0f - factor;
            if(playback.weight <= 0.0f) {
                playback.currentPosition = position;
                return true;
            }
        } else {
            playback.weight = factor;
        }
    }

    return false;
}
/*!
    \internal
*/
bool Animator::updatePosition(PlaybackState &playback, float position) const {
    playback.currentPosition = position;

    if(playback.currentPosition >= 1.0f) {
        if(playback.state->m_loop) {
            while(playback.currentPosition >= 1.0f) {
                playback.currentPosition -= 1.0f;
            }
        } else {
            playback.currentPosition = MIN(playback.currentPosition, 1.0f);
            return true;
        }
    }

    return false;
}
