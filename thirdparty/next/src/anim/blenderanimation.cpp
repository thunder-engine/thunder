#include "anim/blenderanimation.h"

class BlenderAnimationPrivate {
public:
    BlenderAnimationPrivate() :
            m_Factor(0.0f),
            m_Offset(0.0f),
            m_TransitionTime(0.0f),
            m_BeginDuration(0),
            m_EndDuration(0),
            m_LastTime(0) {

    }

    float mix(float value, int32_t component, uint32_t msecs) {
        AnimationCurve *begin = beginCurve(component);
        if(begin) {
            value = MIX(begin->value(msecs), value, m_Factor);
        }
        AnimationCurve *end = endCurve(component);
        if(end) {
            uint32_t shift = static_cast<uint32_t>(m_EndDuration * m_Offset);
            value = MIX(value, end->value(MAX(msecs - shift, 0)), m_Factor);
        }
        return value;
    }

    AnimationCurve *beginCurve(int32_t component) const {
        auto it = m_BeginFrames.find(component);
        if(it != m_BeginFrames.end()) {
            return it->second;
        }
        return nullptr;
    }

    AnimationCurve *endCurve(int32_t component) const {
        auto it = m_EndFrames.find(component);
        if(it != m_EndFrames.end()) {
            return it->second;
        }
        return nullptr;
    }

    unordered_map<int32_t, AnimationCurve *> m_BeginFrames;
    unordered_map<int32_t, AnimationCurve *> m_EndFrames;

    Variant m_Default;

    Variant m_CurrentValue;

    float m_Factor;
    float m_Offset;
    float m_TransitionTime;
    uint32_t m_BeginDuration;
    uint32_t m_EndDuration;
    uint32_t m_LastTime;
};

BlenderAnimation::BlenderAnimation() :
        p_ptr(new BlenderAnimationPrivate()) {

}

BlenderAnimation::~BlenderAnimation() {
    delete p_ptr;
}

Variant BlenderAnimation::defaultValue() {
    return p_ptr->m_Default;
}

void BlenderAnimation::setDefaultValue(const Variant &value) {
    p_ptr->m_Default = value;
    p_ptr->m_CurrentValue = value;
}

/*!
    Sets the new current \a value for the animated Variant.
*/
void BlenderAnimation::setCurrentValue(const Variant &value) {
    p_ptr->m_CurrentValue = value;
}

void BlenderAnimation::setOffset(float offset) {
    p_ptr->m_Offset = offset;
}

void BlenderAnimation::setTransitionTime(float time) {
    p_ptr->m_TransitionTime = time;
    p_ptr->m_Factor = 0.0f;
}

/*!
    Returns the duration of the animation (in milliseconds).
*/
int32_t BlenderAnimation::loopDuration() const {
    uint32_t shift = static_cast<uint32_t>(p_ptr->m_EndDuration * p_ptr->m_Offset);
    return MAX(p_ptr->m_BeginDuration, p_ptr->m_EndDuration + shift);
}

/*!
    \overload
    This function interpolates animated Variant value from one KeyFrame to another.
*/
void BlenderAnimation::setCurrentTime(uint32_t msecs) {
    Animation::setCurrentTime(msecs);
    if(!isValid()) {
        return;
    }

    uint32_t time = loopTime();
    float normalized = static_cast<float>(msecs - p_ptr->m_LastTime) / loopDuration();
    p_ptr->m_LastTime = msecs;

    if(p_ptr->m_TransitionTime > 0.0f) {
        if(p_ptr->m_Factor <= 1.0f) {
            p_ptr->m_Factor += normalized / p_ptr->m_TransitionTime;
        } else {
            p_ptr->m_Factor = 1.0f;
            p_ptr->m_TransitionTime = 0.0f;
        }
    }

    Variant data = p_ptr->m_Default;
    switch(data.type()) {
        case MetaType::BOOLEAN: {
            p_ptr->m_BeginFrames.find(0);
            data = Variant(static_cast<bool>(p_ptr->mix(data.toFloat(), 0, time)));
        } break;
        case MetaType::INTEGER: {
            data = Variant(static_cast<int>(p_ptr->mix(data.toFloat(), 0, time)));
        } break;
        case MetaType::FLOAT: {
            data = Variant(p_ptr->mix(data.toFloat(), 0, time));
        } break;
        case MetaType::VECTOR2: {
            Vector2 v = data.toVector2();
            for(int i = 0; i < 2; i++) {
                v[i] = p_ptr->mix(v[i], i, time);
            }
            data = v;
        } break;
        case MetaType::VECTOR3: {
            Vector3 v = data.toVector3();
            for(int i = 0; i < 3; i++) {
                v[i] = p_ptr->mix(v[i], i, time);
            }
            data = v;
        } break;
        case MetaType::VECTOR4: {
            Vector4 v = data.toVector4();
            for(int i = 0; i < 4; i++) {
                v[i] = p_ptr->mix(v[i], i, time);
            }
            data = v;
        } break;
        default: break;
    }
    setCurrentValue(data);
}

AnimationCurve *BlenderAnimation::beginCurve(int32_t component) const {
    return p_ptr->beginCurve(component);
}

void BlenderAnimation::setBeginCurve(AnimationCurve *curve, int32_t component) {
    if(curve == nullptr) {
        p_ptr->m_BeginFrames.clear();
        p_ptr->m_BeginDuration = 0;
    } else {
        p_ptr->m_BeginFrames[component] = curve;
        p_ptr->m_BeginDuration = curve->duration();
    }
}

AnimationCurve *BlenderAnimation::endCurve(int32_t component) const {
    return p_ptr->endCurve(component);
}

void BlenderAnimation::setEndCurve(AnimationCurve *curve, int32_t component) {
    if(curve == nullptr) {
        p_ptr->m_EndFrames.clear();
        p_ptr->m_EndDuration = 0;
    } else {
        p_ptr->m_EndFrames[component] = curve;
        p_ptr->m_EndDuration = curve->duration();
    }
}
