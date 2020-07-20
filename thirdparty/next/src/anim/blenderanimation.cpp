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

    Quaternion mix(Quaternion &value, uint32_t msecs) {
        AnimationCurve *begin[4];
        begin[0] = beginCurve(0);
        begin[1] = beginCurve(1);
        begin[2] = beginCurve(2);
        begin[3] = beginCurve(3);

        if(begin[0] && begin[1] && begin[2] && begin[3]) {
            value.mix(quaternionValue(begin, msecs), value, m_Factor);
        }

        AnimationCurve *end[4];
        end[0] = beginCurve(0);
        end[1] = beginCurve(1);
        end[2] = beginCurve(2);
        end[3] = beginCurve(3);
        if(end[0] && end[1] && end[2] && end[3]) {
            uint32_t shift = static_cast<uint32_t>(m_EndDuration * m_Offset);
            value.mix(value, quaternionValue(end, MAX(msecs - shift, 0)), m_Factor);
        }
        return value;
    }

    Quaternion quaternionValue(AnimationCurve *curves[4], uint32_t msecs) {
        Quaternion result;
        if(curves[0]->m_Keys.size() >= 2) {
            int32_t begin, end;
            curves[0]->frames(begin, end, msecs);

            if(begin != -1 && end != -1) {
                Quaternion a;
                a.x = curves[0]->m_Keys[begin].m_Value;
                a.y = curves[1]->m_Keys[begin].m_Value;
                a.z = curves[2]->m_Keys[begin].m_Value;
                a.w = curves[3]->m_Keys[begin].m_Value;

                if(begin == end) {
                    return a;
                }

                Quaternion b;
                b.x = curves[0]->m_Keys[end].m_Value;
                b.y = curves[1]->m_Keys[end].m_Value;
                b.z = curves[2]->m_Keys[end].m_Value;
                b.w = curves[3]->m_Keys[end].m_Value;

                uint32_t duration = curves[0]->m_Keys.back().m_Position;
                float f1 = float(curves[0]->m_Keys[begin].m_Position) / duration;
                float f2 = float(curves[0]->m_Keys[end].m_Position) / duration;
                float f  = float(msecs) / duration;
                float factor = (f - f1) / (f2 - f1);

                switch(curves[0]->m_Keys[begin].m_Type) {
                    case AnimationCurve::KeyFrame::Constant: {
                        return (factor >= 1.0f) ? b : a;
                    } break;
                    default: { // Cubic and Linear
                        //result = CMIX(a.m_Value, a.m_RightTangent, b.m_LeftTangent, b.m_Value, factor);
                        result.mix(a, b, factor);
                    } break;
                }
                return result;
            }
        }
        return result;
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

/*!
    \class BlenderAnimation
    \brief The BlenderAnimation class provides mechanism for blending of an animation tracks.
    \since Next 1.0
    \inmodule Animation

    The BlenderAnimation helps to developers to mix animation tracks with various mixing parameters.
*/

BlenderAnimation::BlenderAnimation() :
        p_ptr(new BlenderAnimationPrivate()) {

}

BlenderAnimation::~BlenderAnimation() {
    delete p_ptr;
}

/*!
    Returns the default value for the animated curve. This value will be used if no animation data is available at the current time frame.
*/
Variant BlenderAnimation::defaultValue() {
    return p_ptr->m_Default;
}

/*!
    Sets the default \a value for the animated curve. This value will be used if no animation data is available at the current time frame.
*/
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
        case MetaType::QUATERNION: {
            Animation::setCurrentTime(msecs);
            Quaternion v = data.toQuaternion();
            data = p_ptr->mix(v, loopTime());
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
