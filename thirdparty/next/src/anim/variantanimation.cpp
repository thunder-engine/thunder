#include "anim/variantanimation.h"

class VariantAnimationPrivate {
public:
    map<int32_t, AnimationCurve *> m_KeyFrames;

    Variant         m_CurrentValue;
};
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
        p_ptr(new VariantAnimationPrivate()) {

}

VariantAnimation::~VariantAnimation() {
    delete p_ptr;
}
/*!
    Returns the duration of the animation (in milliseconds).
*/
int32_t VariantAnimation::loopDuration() const {
    uint32_t result = 0;
    for(auto it : p_ptr->m_KeyFrames) {
        result = MAX(it.second->duration(), result);
    }
    return result;
}
/*!
    Returns the current value for the animated Variant.
*/
Variant VariantAnimation::currentValue() const {
    return p_ptr->m_CurrentValue;
}
/*!
    Sets the new current \a value for the animated Variant.
*/
void VariantAnimation::setCurrentValue(const Variant &value) {
    p_ptr->m_CurrentValue   = value;
}

AnimationCurve *VariantAnimation::curve(int32_t component) const {
    auto it = p_ptr->m_KeyFrames.find(component);
    if(it != p_ptr->m_KeyFrames.end()) {
        return it->second;
    }
    return nullptr;
}
/*!
    Sets the new sequence of the key frames as \a curve for the provided \a conponent.
*/
void VariantAnimation::setCurve(AnimationCurve *curve, int32_t component) {
    p_ptr->m_KeyFrames[component] = curve;
}
/*!
    \overload
    This function interpolates animated Variant value from one KeyFrame to another.
*/
void VariantAnimation::setCurrentTime(uint32_t msecs) {
    Animation::setCurrentTime(msecs);
    if(!isValid()) {
        return;
    }
    Variant data = currentValue();
    for(auto it : p_ptr->m_KeyFrames) {
        int32_t component = it.first;

        float value = it.second->value(loopTime());

        switch(p_ptr->m_CurrentValue.type()) {
            case MetaType::BOOLEAN: {
                data = Variant(static_cast<bool>(value));
            } break;
            case MetaType::INTEGER: {
                data = Variant(static_cast<int>(value));
            } break;
            case MetaType::FLOAT: {
                data = Variant(value);
            } break;
            case MetaType::VECTOR2: {
                Vector2 v = data.toVector2();
                v[component] = value;
                data = v;
            } break;
            case MetaType::VECTOR3: {
                Vector3 v = data.toVector3();
                v[component] = value;
                data = v;
            } break;
            case MetaType::VECTOR4: {
                Vector4 v = data.toVector4();
                v[component] = value;
                data = v;
            } break;
            default: break;
        }
    }
    setCurrentValue(data);
}
