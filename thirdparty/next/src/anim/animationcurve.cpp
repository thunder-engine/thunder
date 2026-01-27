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

#include <metatype.h>
#include <float.h>

bool AnimationCurve::KeyFrame::operator ==(const KeyFrame &left) const {
    return abs(m_position - left.m_position) <= FLT_EPSILON  && (m_value == left.m_value);
}

float AnimationCurve::valueFloat(float pos) const {
    int32_t a, b;
    frames(a, b, pos);

    if(a != -1) {
        float factor = (pos - m_keys[a].m_position) / (m_keys[b].m_position - m_keys[a].m_position);

        return value(a, b, 0, factor);
    }
    return (m_keys.empty()) ? 0.0f : m_keys.front().m_value[0];
}

Vector2 AnimationCurve::valueVector2(float pos) const {
    int32_t a, b;
    frames(a, b, pos);

    Vector2 result;
    if(a != -1) {
        float factor = (pos - m_keys[a].m_position) / (m_keys[b].m_position - m_keys[a].m_position);
        for(int i = 0; i < 2; i++) {
            result[i] = value(a, b, i, factor);
        }
    }
    return result;
}

Vector3 AnimationCurve::valueVector3(float pos) const {
    int32_t a, b;
    frames(a, b, pos);

    Vector3 result;
    if(a != -1) {
        float factor = (pos - m_keys[a].m_position) / (m_keys[b].m_position - m_keys[a].m_position);
        for(int i = 0; i < 3; i++) {
            result[i] = value(a, b, i, factor);
        }
    }
    return result;
}

Vector4 AnimationCurve::valueVector4(float pos) const {
    int32_t a, b;
    frames(a, b, pos);

    Vector4 result;
    if(a != -1 && b != -1) {
        float factor = (pos - m_keys[a].m_position) / (m_keys[b].m_position - m_keys[a].m_position);
        for(int i = 0; i < 4; i++) {
            result[i] = value(a, b, i, factor);
        }
    }
    return result;
}

Quaternion AnimationCurve::valueQuaternion(float pos) const {
    int32_t a, b;
    frames(a, b, pos);

    Quaternion result;
    if(a != -1 && b != -1) {
        const KeyFrame &keyA = m_keys[a];
        const KeyFrame &keyB = m_keys[b];

        float factor = (pos - keyA.m_position) / (keyB.m_position - keyA.m_position);

        Quaternion qA(keyA.m_value[0], keyA.m_value[1], keyA.m_value[2], keyA.m_value[3]);
        Quaternion qB(keyB.m_value[0], keyB.m_value[1], keyB.m_value[2], keyB.m_value[3]);
        result.mix(qA, qB, factor);
    }
    return result;
}

float AnimationCurve::value(int32_t a, int32_t b, int c, float f) const {
    const KeyFrame &keyA = m_keys[a];
    const KeyFrame &keyB = m_keys[b];
    if(c >= keyA.m_value.size()) {
        return 0.0f;
    }

    if(a == b) {
        return keyA.m_value[c];
    }

    switch(m_keys[a].m_type) {
        case KeyFrame::Linear: return MIX(keyA.m_value[c], keyB.m_value[c], f); break;
        case KeyFrame::Cubic: return CMIX(keyA.m_value[c], keyA.m_rightTangent[c], keyB.m_leftTangent[c], keyB.m_value[c], f); break;
        default: break;
    }

    return (f >= 0.99f) ? keyB.m_value[c] : keyA.m_value[c];
}

void AnimationCurve::frames(int32_t &b, int32_t &e, float pos) const {
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
