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

    Copyright: 2008-2023 Evgeniy Prikazchikov
*/

#ifndef VECTOR2_H
#define VECTOR2_H

#include <global.h>

class Vector3;
class Vector4;

class NEXT_LIBRARY_EXPORT Vector2 {
public:
    Vector2();
    Vector2(areal v);
    Vector2(areal x, areal y);
    Vector2(const Vector2 &vector);
    Vector2(const Vector3 &vector);
    Vector2(const Vector4 &vector);

    Vector2 &operator=(const Vector2 &value);

    bool operator==(const Vector2 &vector) const;
    bool operator!=(const Vector2 &vector) const;
    bool operator>(const Vector2 &vector) const;
    bool operator<(const Vector2 &vector) const;

    Vector2 operator*(areal factor) const;
    Vector2 operator*(const Vector2 &vector) const;
    Vector2 operator/(areal factor) const;
    Vector2 operator+(const Vector2 &vector) const;
    Vector2 operator-() const;
    Vector2 operator-(const Vector2 &vector) const;

    Vector2 &operator*=(areal factor);
    Vector2 &operator/=(areal divisor);
    Vector2 &operator+=(const Vector2 &vector);
    Vector2 &operator-=(const Vector2 &vector);

    areal &operator[](int i);
    areal operator[](int i) const;

    areal length() const;
    areal sqrLength() const;

    areal normalize();

    areal cross(const Vector2 &vector) const;
    areal dot(const Vector2 &vector) const;

public:
    union {
        struct {
            areal x, y;
        };
        areal v[2];
    };

};

#endif /* VECTOR2_H */
