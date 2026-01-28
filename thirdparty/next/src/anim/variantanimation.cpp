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

    Copyright: 2008-2025 Evgeniy Prikazchikov
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
        \li MetaType::QUATERNION
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

const AnimationCurve &VariantAnimation::curve() const {
    return m_keyFrames;
}
/*!
    Sets the new sequence of the key frames as \a curve.
*/
void VariantAnimation::setCurve(const AnimationCurve &curve) {
    m_keyFrames = curve;
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

    float pos = (float)loopTime() / (float)m_duration;
    switch(m_currentValue.type()) {
        case MetaType::BOOLEAN: setCurrentValue(static_cast<bool>(m_keyFrames.valueFloat(pos))); break;
        case MetaType::INTEGER: setCurrentValue(static_cast<int>(m_keyFrames.valueFloat(pos))); break;
        case MetaType::FLOAT: setCurrentValue(m_keyFrames.valueFloat(pos)); break;
        case MetaType::VECTOR2: setCurrentValue(m_keyFrames.valueVector2(pos)); break;
        case MetaType::VECTOR3: setCurrentValue(m_keyFrames.valueVector3(pos)); break;
        case MetaType::VECTOR4: setCurrentValue(m_keyFrames.valueVector4(pos)); break;
        case MetaType::QUATERNION: {
            Vector4 v(m_keyFrames.valueVector4(pos));
            setCurrentValue(Quaternion(v.x, v.y, v.z, v.w)); break;
        }
        default: break;
    }
}
