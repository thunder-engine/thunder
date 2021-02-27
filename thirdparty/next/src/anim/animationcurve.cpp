#include "anim/animationcurve.h"

AnimationCurve::KeyFrame::KeyFrame() :
    m_Type(Cubic),
    m_Position(0.0f),
    m_Value(0.0f),
    m_LeftTangent(0.0f),
    m_RightTangent(0.0f) {

}

bool AnimationCurve::KeyFrame::operator ==(const KeyFrame &left) {
    return (m_Type == left.m_Type) && (m_Position == left.m_Position) && (m_Value == left.m_Value) &&
           (m_LeftTangent == left.m_LeftTangent) && (m_RightTangent == left.m_RightTangent);
}

float AnimationCurve::value(float pos) {
    float result = (m_Keys.empty()) ? 0.0f : m_Keys.front().m_Value;
    if(m_Keys.size() >= 2) {
        AnimationCurve::KeyFrame a;
        AnimationCurve::KeyFrame b;

        for(auto i = m_Keys.begin(); i != m_Keys.end(); i++) {
            if(pos == i->m_Position) {
                return i->m_Value;
            }
            if(pos >= i->m_Position) {
                a = (*i);
            }
            if(pos <= i->m_Position) {
                b = (*i);
                break;
            }
        }

        float factor = (pos - a.m_Position) / (b.m_Position - a.m_Position);
        switch(a.m_Type) {
            case AnimationCurve::KeyFrame::Constant: {
                result = (factor >= 1.0f) ? b.m_Value : a.m_Value;
            } break;
            case AnimationCurve::KeyFrame::Linear: {
                result = MIX(a.m_Value, b.m_Value, factor);
            } break;
            default: { // Cubic
                result = CMIX(a.m_Value, a.m_RightTangent, b.m_LeftTangent, b.m_Value, factor);
            } break;
        }
    }
    return result;
}

void AnimationCurve::frames(int32_t &b, int32_t &e, float pos) {
    b = e = -1;
    if(m_Keys.size() >= 2) {
        for(uint32_t i = 0; i < m_Keys.size(); i++) {
            if(pos == m_Keys[i].m_Position) {
                b = e = i;
                return;
            }
            if(pos >= m_Keys[i].m_Position) {
                b = i;
            }
            if(pos <= m_Keys[i].m_Position) {
                e = i;
                break;
            }
        }
    }
}
