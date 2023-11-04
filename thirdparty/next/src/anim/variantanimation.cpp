/*
    This file is part of Thunder Next.

    Thunder Next is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

    Thunder Next is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Thunder Next.  If not, see <http://www.gnu.org/licenses/>.

    Copyright: 2008-2023 Evgeniy Prikazchikov
*/

#include "anim/variantanimation.h"

/*!
    \class VariantAnimation
    \brief The VariantAnimation is a base class for all animation tracks.
    \since Next 1.0
    \inmodule Animation

    This class allows to change values in time. VariantAnimation uses key-frame animation mechanism.
    Developers should specify sequence of key values which pair of point in time and key value.
    While animation is playing specific interpolation function moves from one key-frame to another and changing controled value.

    List of supported Variant types for animation:
    \list
        \li MetaType::BOOLEAN
        \li MetaType::INTEGER
        \li MetaType::FLOAT
        \li MetaType::VECTOR2
        \li MetaType::VECTOR3
        \li MetaType::VECTOR4
    \endlist
*/

VariantAnimation::VariantAnimation() :
        m_duration(-1) {

}

VariantAnimation::~VariantAnimation() {

}
/*!
    Returns the duration of the animation (in milliseconds).
*/
int32_t VariantAnimation::duration() const {
    return m_duration;
}
/*!
    Sets a new \a duration of the animation in milliseconds.
*/
void VariantAnimation::setDuration(int32_t duration) {
    if(duration < 0 || m_duration == duration) {
       return;
    }
    m_duration = duration;
}
/*!
    Returns the current value for the animated Variant.
*/
Variant VariantAnimation::currentValue() const {
    return m_currentValue;
}
/*!
    Sets the new current \a value for the animated Variant.
*/
void VariantAnimation::setCurrentValue(const Variant &value) {
    m_currentValue = value;
}

AnimationCurve *VariantAnimation::curve(int32_t component) const {
    auto it = m_keyFrames.find(component);
    if(it != m_keyFrames.end()) {
        return it->second;
    }
    return nullptr;
}
/*!
    Sets the new sequence of the key frames as \a curve for the provided \a component.
*/
void VariantAnimation::setCurve(AnimationCurve *curve, int32_t component) {
    m_keyFrames[component] = curve;
}
/*!
    \overload
    This function interpolates animated Variant value from one KeyFrame to another at \a position in milliseconds.
*/
void VariantAnimation::setCurrentTime(uint32_t position) {
    Animation::setCurrentTime(position);
    if(!isValid()) {
        return;
    }
    float time = (float)loopTime() / (float)m_duration;
    Variant data = currentValue();
    switch(m_currentValue.type()) {
        case MetaType::BOOLEAN: {
            for(auto it : m_keyFrames) {
                float value = it.second->value(time);
                data = Variant(static_cast<bool>(value));
            }
        } break;
        case MetaType::INTEGER: {
            for(auto it : m_keyFrames) {
                float value = it.second->value(time);
                data = Variant(static_cast<int>(value));
            }
        } break;
        case MetaType::FLOAT: {
            for(auto it : m_keyFrames) {
                float value = it.second->value(time);
                data = Variant(value);
            }
        } break;
        case MetaType::VECTOR2: {
            Vector2 v = data.toVector2();
            for(auto it : m_keyFrames) {
                int32_t component = it.first;
                float value = it.second->value(time);
                v[component] = value;
            }
            data = v;
        } break;
        case MetaType::VECTOR3: {
            Vector3 v = data.toVector3();
            for(auto it : m_keyFrames) {
                int32_t component = it.first;
                float value = it.second->value(time);
                v[component] = value;
            }
            data = v;
        } break;
        case MetaType::VECTOR4: {
            Vector4 v = data.toVector4();
            for(auto it : m_keyFrames) {
                int32_t component = it.first;
                float value = it.second->value(time);
                v[component] = value;
            }
            data = v;
        } break;
        case MetaType::QUATERNION: {
            AnimationCurve *corves[4];
            corves[0] = curve(0);
            corves[1] = curve(1);
            corves[2] = curve(2);
            corves[3] = curve(3);
            data = quaternionValue(corves, time);
        } break;
        default: break;
    }
    setCurrentValue(data);
}

Quaternion VariantAnimation::quaternionValue(AnimationCurve *curves[4], float posintion) {
    PROFILE_FUNCTION();

    Quaternion result;
    if(curves[0]->m_Keys.size() >= 2) {
        int32_t begin, end;
        curves[0]->frames(begin, end, posintion);

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
            float f  = float(posintion) / duration;
            float factor = (f - f1) / (f2 - f1);

            switch(curves[0]->m_Keys[begin].m_Type) {
                case AnimationCurve::KeyFrame::Constant: {
                    return (factor >= 1.0f) ? b : a;
                } break;
                case AnimationCurve::KeyFrame::Linear: {
                    result.mix(a, b, factor);
                } break;
                default: { // Cubic
                    //result = CMIX(a.m_Value, a.m_RightTangent, b.m_LeftTangent, b.m_Value, factor);
                } break;
            }
        }
    }
    return result;
}
