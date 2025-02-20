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

#include "anim/animationcurve.h"

#include <float.h>

bool AnimationCurve::KeyFrame::operator ==(const KeyFrame &left) {
    return abs(m_position - left.m_position) <= FLT_EPSILON  && (m_value == left.m_value);
}

Variant AnimationCurve::value(float pos) const {
    Variant result = (m_keys.empty()) ? 0.0f : m_keys.front().m_value;
    if(m_keys.size() >= 2) {
        AnimationCurve::KeyFrame a;
        AnimationCurve::KeyFrame b;

        for(auto i = m_keys.begin(); i != m_keys.end(); i++) {
            if(pos == i->m_position) {
                return i->m_value;
            }
            if(pos >= i->m_position) {
                a = (*i);
            }
            if(pos <= i->m_position) {
                b = (*i);
                break;
            }
        }

        float factor = (pos - a.m_position) / (b.m_position - a.m_position);

        switch(a.m_value.type()) {
            case MetaType::INTEGER: {
                switch(a.m_type) {
                    case AnimationCurve::KeyFrame::Linear: return MIX(a.m_value.toInt(), b.m_value.toInt(), factor); break;
                    case AnimationCurve::KeyFrame::Cubic: return CMIX(a.m_value.toInt(), a.m_rightTangent, b.m_leftTangent, b.m_value.toInt(), factor); break;
                    default: break;
                }
            } break;
            case MetaType::FLOAT: {
                switch(a.m_type) {
                    case AnimationCurve::KeyFrame::Linear: return MIX(a.m_value.toFloat(), b.m_value.toFloat(), factor); break;
                    case AnimationCurve::KeyFrame::Cubic: return CMIX(a.m_value.toFloat(), a.m_rightTangent, b.m_leftTangent, b.m_value.toFloat(), factor); break;
                    default: break;
                }
            } break;
            case MetaType::VECTOR2: {
                Vector2 av(a.m_value.toVector2());
                Vector2 bv(b.m_value.toVector2());

                switch(a.m_type) {
                    case AnimationCurve::KeyFrame::Linear: {
                        return Vector2(MIX(av.x, bv.x, factor), MIX(av.y, bv.y, factor));
                    } break;
                    case AnimationCurve::KeyFrame::Cubic: {
                        return Vector2(CMIX(av.x, a.m_rightTangent, b.m_leftTangent, bv.x, factor),
                                       CMIX(av.y, a.m_rightTangent, b.m_leftTangent, bv.y, factor));
                    } break;
                    default: break;
                }
            } break;
            case MetaType::VECTOR3: {
                Vector3 av(a.m_value.toVector3());
                Vector3 bv(b.m_value.toVector3());

                switch(a.m_type) {
                    case AnimationCurve::KeyFrame::Linear: {
                        return Vector3(MIX(av.x, bv.x, factor), MIX(av.y, bv.y, factor), MIX(av.z, bv.z, factor));
                    } break;
                    case AnimationCurve::KeyFrame::Cubic: {
                        return Vector3(CMIX(av.x, a.m_rightTangent, b.m_leftTangent, bv.x, factor),
                                       CMIX(av.y, a.m_rightTangent, b.m_leftTangent, bv.y, factor),
                                       CMIX(av.z, a.m_rightTangent, b.m_leftTangent, bv.z, factor));
                    } break;
                    default: break;
                }
            } break;
            case MetaType::VECTOR4: {
                Vector4 av(a.m_value.toVector4());
                Vector4 bv(b.m_value.toVector4());

                switch(a.m_type) {
                    case AnimationCurve::KeyFrame::Linear: {
                        return Vector4(MIX(av.x, bv.x, factor), MIX(av.y, bv.y, factor), MIX(av.z, bv.z, factor), MIX(av.w, bv.w, factor));
                    } break;
                    case AnimationCurve::KeyFrame::Cubic: {
                        return Vector4(CMIX(av.x, a.m_rightTangent, b.m_leftTangent, bv.x, factor),
                                       CMIX(av.y, a.m_rightTangent, b.m_leftTangent, bv.y, factor),
                                       CMIX(av.z, a.m_rightTangent, b.m_leftTangent, bv.z, factor),
                                       CMIX(av.w, a.m_rightTangent, b.m_leftTangent, bv.w, factor));
                    } break;
                    default: break;
                }
            } break;
            case MetaType::QUATERNION: {
                Quaternion aq(a.m_value.toQuaternion());
                Quaternion bq(b.m_value.toQuaternion());
                switch(a.m_type) {
                    case AnimationCurve::KeyFrame::Linear: {
                        Quaternion q;
                        q.mix(aq, bq, factor);
                        return q;
                    } break;
                    default: break;
                }
            } break;
            default: break;
        }
        result = (factor >= 0.99f) ? b.m_value : a.m_value;
    }
    return result;
}

void AnimationCurve::frames(int32_t &b, int32_t &e, float pos) {
    b = e = -1;
    if(m_keys.size() >= 2) {
        for(uint32_t i = 0; i < m_keys.size(); i++) {
            if(pos == m_keys[i].m_position) {
                b = e = i;
                return;
            }
            if(pos >= m_keys[i].m_position) {
                b = i;
            }
            if(pos <= m_keys[i].m_position) {
                e = i;
                break;
            }
        }
    }
}
