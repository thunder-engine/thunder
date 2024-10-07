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

#ifndef QUATERNION_H
#define QUATERNION_H

#include <global.h>

class Vector3;
class Matrix3;

class NEXT_LIBRARY_EXPORT Quaternion {
public:
    Quaternion();
    Quaternion(areal x, areal y, areal z, areal w);
    Quaternion(const Vector3 &axis, areal angle);
    Quaternion(const Vector3 &euler);
    Quaternion(const Matrix3 &matrix);

    bool operator==(const Quaternion &quaternion) const;
    bool operator!=(const Quaternion &quaternion) const;

    Quaternion operator*(areal factor) const;
    Quaternion operator*(const Quaternion &quaternion) const;
    Vector3 operator*(const Vector3 &vector) const;
    Quaternion operator/(areal divisor) const;

    Quaternion &operator*=(areal factor);
    Quaternion &operator/=(areal divisor);

    areal &operator[](int i);
    areal operator[](int i) const;

    areal length() const;
    areal sqrLength() const;

    areal normalize();

    areal dot(const Quaternion &quaternion) const;

    bool equal(const Quaternion &quaternion) const;

    Quaternion inverse() const;

    void mix(const Quaternion &q0, const Quaternion &q1, areal t);

    Matrix3 toMatrix() const;
    Vector3 euler() const;

    void axisAngle(Vector3 &axis, areal &angle);

    static Quaternion lookRotation(const Vector3 &forward, const Vector3 &up);

public:
    union {
        struct {
            areal x, y, z, w;
        };
        areal q[4];
    };

};

#endif /* QUATERNION_H */
