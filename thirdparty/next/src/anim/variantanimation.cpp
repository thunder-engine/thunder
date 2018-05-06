#include "anim/variantanimation.h"

class VariantAnimationPrivate {
public:
    VariantAnimationPrivate() :
            m_Duration(0) {
    }

    FrameVector             m_KeyFrames;
    Variant                 m_CurrentValue;
    int32_t                 m_Duration;
};

VariantAnimation::VariantAnimation() :
        p_ptr(new VariantAnimationPrivate()) {

}

VariantAnimation::~VariantAnimation() {
    delete p_ptr;
}

int32_t VariantAnimation::loopDuration() const {
    return p_ptr->m_Duration;
}

void VariantAnimation::setLoopDuration(int32_t msec) {
    p_ptr->m_Duration   = msec;
}

Variant VariantAnimation::currentValue() const {
    return p_ptr->m_CurrentValue;
}

void VariantAnimation::setKeyFrames(const FrameVector &frames) {
    p_ptr->m_KeyFrames  = frames;
}

void VariantAnimation::update() {
    float factor    = float(loopTime()) / float(loopDuration());

    if(p_ptr->m_KeyFrames.size() >= 2) {
        KeyFrame a;
        KeyFrame b;
        for(size_t i = 0; i < p_ptr->m_KeyFrames.size(); i++) {
            if(factor == p_ptr->m_KeyFrames[i].first) {
                p_ptr->m_CurrentValue   = p_ptr->m_KeyFrames[i].second;
                return;
            }
            if(factor >= p_ptr->m_KeyFrames[i].first) {
                a   = p_ptr->m_KeyFrames[i];
            }
            if(factor <= p_ptr->m_KeyFrames[i].first) {
                b   = p_ptr->m_KeyFrames[i];
                break;
            }
        }
        factor  = (factor - a.first) / (b.first - a.first);

        if(a.second.type() == b.second.type()) {
            switch(a.second.type()) {
                case MetaType::INTEGER: {
                    valueUpdated(MIX(a.second.toInt(),    b.second.toInt(), factor));
                } break;
                case MetaType::FLOAT: {
                    valueUpdated(MIX(a.second.toFloat(),  b.second.toInt(), factor));
                } break;
                case MetaType::VECTOR2: {
                    valueUpdated(MIX(a.second.toVector2(), b.second.toVector2(), factor));
                } break;
                case MetaType::VECTOR3: {
                    valueUpdated(MIX(a.second.toVector3(), b.second.toVector3(), factor));
                } break;
                case MetaType::VECTOR4: {
                    valueUpdated(MIX(a.second.toVector4(), b.second.toVector4(), factor));
                } break;
                case MetaType::QUATERNION: {
                    Quaternion result;
                    result.mix(a.second.toQuaternion(), b.second.toQuaternion(), factor);
                    valueUpdated(result);
                } break;
                default: break;
            }
        }
    }
}

void VariantAnimation::valueUpdated(const Variant &value) {
    p_ptr->m_CurrentValue   = value;
}
