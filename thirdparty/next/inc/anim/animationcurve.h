#ifndef ANIMATIONCURVE_H
#define ANIMATIONCURVE_H

#include "variant.h"

class NEXT_LIBRARY_EXPORT AnimationCurve {
public:
    class NEXT_LIBRARY_EXPORT KeyFrame {
    public:
        enum Type {
            Constant = 0,
            Linear,
            Cubic
        };

        KeyFrame ();

        bool operator ==(const KeyFrame &left);

    public:
        Type m_Type;

        float m_Position;
        float m_Value;

        float m_LeftTangent;
        float m_RightTangent;
    };

    typedef vector<KeyFrame> Keys;

    float value(float pos);

    void frames(int32_t &b, int32_t &e, float pos);

    Keys m_Keys;
};

#endif // ANIMATIONCURVE_H
