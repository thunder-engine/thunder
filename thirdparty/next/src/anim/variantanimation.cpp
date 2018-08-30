#include "anim/variantanimation.h"

class VariantAnimationPrivate {
public:
    VariantAnimationPrivate() :
            m_KeyFrames(nullptr) {
    }

    VariantAnimation::Curve    *m_KeyFrames;
    Variant                     m_CurrentValue;
};
/*!
    \class VariantAnimation
    \brief The VariantAnimation is a base class for all animation tracks.
    \since Next 1.0
    \inmodule Anim

    This class allows to change values in time. VariantAnimation uses key-frame animation mechanism.
    Developers should specify sequence of key values wich pair of point in time and key value.
    While animation is playing specific interpolation function moves from one key-frame to another and changing controled value.

    List of supported Variant types for animation:
    \list
        \li MetaType::INTEGER
        \li MetaType::FLOAT
        \li MetaType::VECTOR2
        \li MetaType::VECTOR3
        \li MetaType::VECTOR4
        \li MetaType::QUATERNION
    \endlist
*/
/*!
    Constructs VariantAnimation object
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
    if(p_ptr->m_KeyFrames && !p_ptr->m_KeyFrames->empty()) {
        return p_ptr->m_KeyFrames->back().mPosition;
    }
    return 0;
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
/*!
    Returns the sequence of key frames as curve for the animation track.
*/
VariantAnimation::Curve &VariantAnimation::keyFrames() const {
    return *(p_ptr->m_KeyFrames);
}
/*!
    Sets the new sequence of the key \a frames.
*/
void VariantAnimation::setKeyFrames(Curve &frames) {
    p_ptr->m_KeyFrames  = &frames;
}
/*!
    \overload
    This function interpolates animated Variant value from one KeyFrame to another.
*/
void VariantAnimation::update() {
    uint32_t duration   = loopDuration();
    uint32_t current    = loopTime();

    if(p_ptr->m_KeyFrames->size() >= 2) {
        KeyFrame a;
        KeyFrame b;

        for(auto i = p_ptr->m_KeyFrames->begin(); i != p_ptr->m_KeyFrames->end(); i++) {
            if(current == i->mPosition) {
                setCurrentValue(i->mValue);
                return;
            }
            if(current >= i->mPosition) {
                a   = (*i);
            }
            if(current <= i->mPosition) {
                b   = (*i);
                break;
            }
        }
        float f1    = float(a.mPosition) / duration;
        float f2    = float(b.mPosition) / duration;
        float f     = float(current) / duration;
        float factor    = (f - f1) / (f2 - f1);

        if(a.mValue.type() == b.mValue.type()) {
            switch(a.mValue.type()) {
                case MetaType::BOOLEAN: {
                    setCurrentValue((factor >= 1.0f) ? b.mValue.toBool() : a.mValue.toBool());
                } break;
                case MetaType::INTEGER: {
                    if(a.mType == KeyFrame::Linear) {
                        setCurrentValue(MIX(a.mValue.toInt(), b.mValue.toInt(), factor));
                    } else {
                        setCurrentValue(CMIX(a.mValue.toInt(), a.mSupport.toInt(), b.mSupport.toInt(), b.mValue.toInt(), factor));
                    }
                } break;
                case MetaType::FLOAT: {
                    if(a.mType == KeyFrame::Linear) {
                        setCurrentValue(MIX(a.mValue.toFloat(), b.mValue.toFloat(), factor));
                    } else {
                        setCurrentValue(CMIX(a.mValue.toFloat(), a.mSupport.toFloat(), b.mSupport.toFloat(), b.mValue.toFloat(), factor));
                    }
                } break;
                case MetaType::VECTOR2: {
                    if(a.mType == KeyFrame::Linear) {
                        setCurrentValue(MIX(a.mValue.toVector2(), b.mValue.toVector2(), factor));
                    } else {
                        setCurrentValue(CMIX(a.mValue.toVector2(), a.mSupport.toVector2(), b.mSupport.toVector2(), b.mValue.toVector2(), factor));
                    }
                } break;
                case MetaType::VECTOR3: {
                    if(a.mType == KeyFrame::Linear) {
                        setCurrentValue(MIX(a.mValue.toVector3(), b.mValue.toVector3(), factor));
                    } else {
                        setCurrentValue(CMIX(a.mValue.toVector3(), a.mSupport.toVector3(), b.mSupport.toVector3(), b.mValue.toVector3(), factor));
                    }
                } break;
                case MetaType::VECTOR4: {
                    if(a.mType == KeyFrame::Linear) {
                        setCurrentValue(MIX(a.mValue.toVector4(), b.mValue.toVector4(), factor));
                    } else {
                        setCurrentValue(CMIX(a.mValue.toVector4(), a.mSupport.toVector4(), b.mSupport.toVector4(), b.mValue.toVector4(), factor));
                    }
                } break;
                case MetaType::QUATERNION: {
                    Quaternion result;
                    result.mix(a.mValue.toQuaternion(), b.mValue.toQuaternion(), factor);
                    setCurrentValue(result);
                } break;
                default: break;
            }
        }
    }
}
