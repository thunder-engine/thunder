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

        bool operator ==(const KeyFrame &left) const;

    public:
        Type m_type = Cubic;

        float m_position = 0.0f;
        Variant m_value;

        float m_leftTangent = 0.0f;
        float m_rightTangent = 0.0f;
    };

    typedef std::vector<KeyFrame> Keys;

    Variant value(float pos) const;

    void frames(int32_t &b, int32_t &e, float pos);

    Keys m_keys;
};

#endif // ANIMATIONCURVE_H
