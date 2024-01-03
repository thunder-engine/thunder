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

#ifndef VECTOR3_H
#define VECTOR3_H

#include <global.h>

class Vector2;

class NEXT_LIBRARY_EXPORT Vector3 {
public:
    Vector3();
    Vector3(areal v);
    Vector3(areal x, areal y, areal z);
    Vector3(const Vector2 &vector, areal z);
    Vector3(const areal *v);
    Vector3(const Vector3 &value);

    Vector3 &operator=(const Vector3 &vector);

    bool operator==(const Vector3 &vector) const;
    bool operator!=(const Vector3 &vector) const;
    bool operator>(const Vector3 &vector) const;
    bool operator<(const Vector3 &vector) const;

    Vector3 operator*(areal factor) const;
    Vector3 operator*(const Vector3 &vector) const;
    Vector3 operator/(areal divisor) const;
    Vector3 operator+(const Vector3 &vector) const;
    Vector3 operator-() const;
    Vector3 operator-(const Vector3 &vector) const;

    Vector3 &operator*=(areal factor);
    Vector3 &operator/=(areal divisor);
    Vector3 &operator+=(const Vector3 &vector);
    Vector3 &operator-=(const Vector3 &vector);
    
    areal &operator[](int i);
    areal operator[](int i) const;

    areal length() const;
    areal sqrLength() const;

    areal normalize();

    Vector3 cross(const Vector3 &vector) const;
    areal dot(const Vector3 &vector) const;

    Vector3 abs() const;

    areal angle(const Vector3 &vector) const;
    areal signedAngle(const Vector3 &vector, const Vector3 up) const;

public:
    union {
        struct {
            areal x, y, z;
        };
        areal v[3];
    };

};

#endif /* VECTOR3_H */
