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

#ifndef VECTOR4_H
#define VECTOR4_H

#include <global.h>

class Vector2;
class Vector3;

class NEXT_LIBRARY_EXPORT Vector4 {
public:
    Vector4();
    Vector4(areal v);
    Vector4(areal x, areal y, areal z, areal w);
    Vector4(const Vector2 &vector, areal z, areal w);
    Vector4(const Vector3 &vector, areal w);
    Vector4(const Vector4 &vector);

    Vector4 &operator=(const Vector4 &value);

    bool operator==(const Vector4 &vector) const;
    bool operator!=(const Vector4 &vector) const;
    bool operator>(const Vector4 &vector) const;
    bool operator<(const Vector4 &vector) const;

    Vector4 operator*(areal factor) const;
    Vector4 operator*(const Vector4 &vector) const;
    Vector4 operator/(areal divisor) const;
    Vector4 operator+(const Vector4 &vector) const;
    Vector4 operator-() const;
    Vector4 operator-(const Vector4 &vector) const;

    Vector4 &operator*=(areal factor);
    Vector4 &operator/=(areal divisor);
    Vector4 &operator+=(const Vector4 &vector);
    Vector4 &operator-=(const Vector4 &vector);

    areal &operator[](int i);
    areal operator[](int i) const;

    areal length() const;
    areal sqrLength() const;

    areal normalize();

    areal dot(const Vector4 &vector) const;

public:
    union {
        struct {
            areal x, y, z, w;
        };
        areal v[4];
    };

};

#endif /* VECTOR4_H */
