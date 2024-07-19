#include "private/baseanimationblender.h"

#include "animationclip.h"

BaseAnimationBlender::BaseAnimationBlender() :
        m_previousTrack(nullptr),
        m_factor(0.0f),
        m_offset(0.0f),
        m_transitionTime(0.0f),
        m_previousDuration(0),
        m_previousTime(0) {

}

void BaseAnimationBlender::setOffset(float offset) {
    m_offset = offset;
}

void BaseAnimationBlender::setTransitionTime(float time) {
    m_transitionTime = time;
    m_factor = 0.0f;
}

int32_t BaseAnimationBlender::duration() const {
    PROFILE_FUNCTION();

    int32_t endDuration = PropertyAnimation::duration();
    uint32_t shift = static_cast<uint32_t>(endDuration * m_offset);
    return MAX(m_previousDuration, endDuration + shift);
}

void BaseAnimationBlender::setPreviousDuration(int32_t duration) {
    PROFILE_FUNCTION();

    m_previousDuration = duration;
}

void BaseAnimationBlender::setCurrentTime(uint32_t position) {
    PROFILE_FUNCTION();

    if(!isValid()) {
        return;
    }

    if(state() == RUNNING) {
        Animation::setCurrentTime(position);

        float time = static_cast<float>(loopTime()) / static_cast<float>(duration());

        float normalized = static_cast<float>(position - m_previousTime) / duration();
        if(m_transitionTime > 0.0f) {
            if(m_factor <= 1.0f) {
                m_factor += normalized / m_transitionTime;
            } else {
                m_factor = 1.0f;
                m_transitionTime = 0.0f;
            }
        }

        Variant data = defaultValue();

        if(m_previousTrack) {
            data = m_previousTrack->curve().value(time);
        }

        Variant target = curve().value(MAX(time - m_offset, 0.0f));

        switch(data.type()) {
            case MetaType::BOOLEAN: {
                 data = target.toBool();
            } break;
            case MetaType::INTEGER: {
                data = MIX(target.toInt(), data.toInt(), m_factor);
            } break;
            case MetaType::FLOAT: {
                data = MIX(target.toFloat(), data.toFloat(), m_factor);
            } break;
            case MetaType::VECTOR2: {
                Vector2 v = data.toVector2();
                Vector2 t = target.toVector2();
                for(int i = 0; i < 2; i++) {
                    v[i] = MIX(t[i], v[i], m_factor);
                }
                data = v;
            } break;
            case MetaType::VECTOR3: {
                Vector3 v = data.toVector3();
                Vector3 t = target.toVector3();
                for(int i = 0; i < 3; i++) {
                    v[i] = MIX(t[i], v[i], m_factor);
                }
                data = v;
            } break;
            case MetaType::VECTOR4: {
                Vector4 v = data.toVector4();
                Vector4 t = target.toVector4();
                for(int i = 0; i < 4; i++) {
                    v[i] = MIX(t[i], v[i], m_factor);
                }
                data = v;
            } break;
            default: data = target; break;
        }
        setCurrentValue(data);

        m_previousTime = position;
    }
}

void BaseAnimationBlender::setPreviousTrack(AnimationTrack &track) {
    PROFILE_FUNCTION();

    m_previousTrack = &track;
}
