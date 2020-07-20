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

    public:
        uint32_t m_Position;

        Type m_Type;

        float m_Value;

        float m_LeftTangent;
        float m_RightTangent;
    };

    typedef vector<KeyFrame> Keys;

    float value (uint32_t pos);

    void frames(int32_t &b, int32_t &e, uint32_t pos);

    uint32_t duration ();

    Keys m_Keys;
};

#endif // ANIMATIONCURVE_H
