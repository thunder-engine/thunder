#include "private/baseanimationblender.h"

BaseAnimationBlender::BaseAnimationBlender() :
        m_Factor(0.0f),
        m_Offset(0.0f),
        m_TransitionTime(0.0f),
        m_PrevDuration(0),
        m_PreviousTime(0) {

}

void BaseAnimationBlender::setOffset(float offset) {
    m_Offset = offset;
}

void BaseAnimationBlender::setTransitionTime(float time) {
    m_TransitionTime = time;
    m_Factor = 0.0f;
}

int32_t BaseAnimationBlender::duration() const {
    PROFILE_FUNCTION();

    int32_t endDuration = PropertyAnimation::duration();
    uint32_t shift = static_cast<uint32_t>(endDuration * m_Offset);
    return MAX(m_PrevDuration, endDuration + shift);
}

void BaseAnimationBlender::setPreviousDuration(int32_t duration) {
    PROFILE_FUNCTION();

    m_PrevDuration = duration;
}

void BaseAnimationBlender::setCurrentTime(uint32_t position) {
    PROFILE_FUNCTION();

    if(!isValid()) {
        return;
    }
    Animation::setCurrentTime(position);
    if(state() == RUNNING) {
        float time = static_cast<float>(loopTime()) / static_cast<float>(duration());

        float normalized = static_cast<float>(position - m_PreviousTime) / duration();
        if(m_TransitionTime > 0.0f) {
            if(m_Factor <= 1.0f) {
                m_Factor += normalized / m_TransitionTime;
            } else {
                m_Factor = 1.0f;
                m_TransitionTime = 0.0f;
            }
        }

        Variant data = defaultValue();
        switch(data.type()) {
            case MetaType::BOOLEAN: {
                 data = Variant(static_cast<bool>(mix(data.toFloat(), 0, time)));
            } break;
            case MetaType::INTEGER: {
                data = Variant(static_cast<int>(mix(data.toFloat(), 0, time)));
            } break;
            case MetaType::FLOAT: {
                data = Variant(mix(data.toFloat(), 0, time));
            } break;
            case MetaType::VECTOR2: {
                Vector2 v = data.toVector2();
                for(int i = 0; i < 2; i++) {
                    v[i] = mix(v[i], i, time);
                }
                data = v;
            } break;
            case MetaType::VECTOR3: {
                Vector3 v = data.toVector3();
                for(int i = 0; i < 3; i++) {
                    v[i] = mix(v[i], i, time);
                }
                data = v;
            } break;
            case MetaType::VECTOR4: {
                Vector4 v = data.toVector4();
                for(int i = 0; i < 4; i++) {
                    v[i] = mix(v[i], i, time);
                }
                data = v;
            } break;
            case MetaType::QUATERNION: {
                Quaternion v = data.toQuaternion();
                data = mix(v, time);
            } break;
            default: break;
        }
        setCurrentValue(data);

        m_PreviousTime = position;
    }
}

AnimationCurve *BaseAnimationBlender::previousCurve(int32_t component) const {
    PROFILE_FUNCTION();

    auto it = m_PrevCurve.find(component);
    if(it != m_PrevCurve.end()) {
        return it->second;
    }
    return nullptr;
}

void BaseAnimationBlender::setPreviousCurve(AnimationCurve *curve, int32_t component) {
    PROFILE_FUNCTION();

    if(curve == nullptr) {
        m_PrevCurve.clear();
    } else {
        m_PrevCurve[component] = curve;
    }
    m_PrevDuration = 0;
}

float BaseAnimationBlender::mix(float value, int32_t component, float position) {
    PROFILE_FUNCTION();

    AnimationCurve *begin = previousCurve(component);
    if(begin) {
        value = begin->value(position);
    }

    AnimationCurve *end = curve(component);
    if(end) {
        value = MIX(end->value(MAX(position - m_Offset, 0.0f)), value, m_Factor);
    }
    return value;
}

Quaternion BaseAnimationBlender::mix(Quaternion &value, float position) {
    PROFILE_FUNCTION();

    AnimationCurve *begin[4];
    begin[0] = previousCurve(0);
    begin[1] = previousCurve(1);
    begin[2] = previousCurve(2);
    begin[3] = previousCurve(3);

    if(begin[0] && begin[1] && begin[2] && begin[3]) {
        value = quaternionValue(begin, position);
    }

    AnimationCurve *end[4];
    end[0] = curve(0);
    end[1] = curve(1);
    end[2] = curve(2);
    end[3] = curve(3);
    if(end[0] && end[1] && end[2] && end[3]) {
        value.mix(quaternionValue(end, MAX(position - m_Offset, 0.0f)), value, m_Factor);
    }
    return value;
}
