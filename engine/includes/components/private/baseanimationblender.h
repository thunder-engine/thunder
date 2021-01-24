#ifndef BASEANIMATIONBLENDER_H
#define BASEANIMATIONBLENDER_H

class BaseAnimationBlender : public PropertyAnimation {
public:
    BaseAnimationBlender() :
            m_Factor(0.0f),
            m_Offset(0.0f),
            m_TransitionTime(0.0f),
            m_BeginDuration(0),
            m_EndDuration(0),
            m_LastTime(0) {

    }

    void setOffset(float offset) {
        m_Offset = offset;
    }

    void setTransitionTime(float time) {
        m_TransitionTime = time;
        m_Factor = 0.0f;
    }

    int32_t loopDuration() const {
        uint32_t shift = static_cast<uint32_t>(m_EndDuration * m_Offset);
        return MAX(m_BeginDuration, m_EndDuration + shift);
    }

    void setCurrentTime(uint32_t posintion) {
        PROFILE_FUNCTION();

        if(!isValid()) {
            return;
        }
        Animation::setCurrentTime(posintion);
        if(state() == RUNNING) {
            uint32_t time = loopTime();
            float normalized = static_cast<float>(posintion - m_LastTime) / loopDuration();
            m_LastTime = posintion;

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
                    m_BeginFrames.find(0);
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
                    Animation::setCurrentTime(posintion);
                    Quaternion v = data.toQuaternion();
                    data = mix(v, loopTime());
                } break;
                default: break;
            }
            setCurrentValue(data);
        }
    }

    AnimationCurve *beginCurve(int32_t component) const {
        PROFILE_FUNCTION();

        auto it = m_BeginFrames.find(component);
        if(it != m_BeginFrames.end()) {
            return it->second;
        }
        return nullptr;
    }

    void setBeginCurve(AnimationCurve *curve, int32_t component = 0) {
        PROFILE_FUNCTION();

        if(curve == nullptr) {
            m_BeginFrames.clear();
            m_BeginDuration = 0;
        } else {
            m_BeginFrames[component] = curve;
            m_BeginDuration = curve->duration();
        }
    }

    AnimationCurve *endCurve(int32_t component) const {
        PROFILE_FUNCTION();

        auto it = m_EndFrames.find(component);
        if(it != m_EndFrames.end()) {
            return it->second;
        }
        return nullptr;
    }

    void setEndCurve(AnimationCurve *curve, int32_t component = 0) {
        PROFILE_FUNCTION();

        if(curve == nullptr) {
            m_EndFrames.clear();
            m_EndDuration = 0;
        } else {
            m_EndFrames[component] = curve;
            m_EndDuration = curve->duration();
        }
    }

private:
    float mix(float value, int32_t component, uint32_t msecs) {
        PROFILE_FUNCTION();

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
        PROFILE_FUNCTION();

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
        PROFILE_FUNCTION();

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
            }
        }
        return result;
    }

    unordered_map<int32_t, AnimationCurve *> m_BeginFrames;
    unordered_map<int32_t, AnimationCurve *> m_EndFrames;

    float m_Factor;
    float m_Offset;
    float m_TransitionTime;
    uint32_t m_BeginDuration;
    uint32_t m_EndDuration;
    uint32_t m_LastTime;
};

#endif //BASEANIMATIONBLENDER_H
