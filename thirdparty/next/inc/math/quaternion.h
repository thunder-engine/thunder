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
    Quaternion(const Quaternion &quaternion);

    Quaternion &operator=(const Quaternion &value);

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

#endif // QUATERNION_H
