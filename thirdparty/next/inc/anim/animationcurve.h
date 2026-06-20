/*
    This file is part of Thunder Next.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#ifndef ANIMATIONCURVE_H
#define ANIMATIONCURVE_H

#include <amath.h>

#include <vector>

class NEXT_LIBRARY_EXPORT AnimationCurve {
public:
    class NEXT_LIBRARY_EXPORT KeyFrame {
    public:
        enum Type {
            Constant = 0,
            Linear,
            Cubic
        };

        bool operator ==(const KeyFrame &left) const;

    public:
        Type m_type = Cubic;

        float m_position = 0.0f;
        std::vector<float> m_value;

        std::vector<float> m_leftTangent;
        std::vector<float> m_rightTangent;
    };

    typedef std::vector<KeyFrame> Keys;

public:
    float valueFloat(float pos) const;
    Vector2 valueVector2(float pos) const;
    Vector3 valueVector3(float pos) const;
    Vector4 valueVector4(float pos) const;
    Quaternion valueQuaternion(float pos) const;

    Keys m_keys;

protected:
    float value(int32_t a, int32_t b, int c, float f) const;

    void frames(int32_t &b, int32_t &e, float pos) const;

};

#endif // ANIMATIONCURVE_H
