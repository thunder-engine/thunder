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

#ifndef VECTOR3_H
#define VECTOR3_H

#include <global.h>

class Vector2;
class Vector4;

class NEXT_LIBRARY_EXPORT Vector3 {
public:
    Vector3();
    Vector3(areal v);
    Vector3(areal x, areal y, areal z);
    Vector3(const Vector2 &vector, areal z);
    Vector3(const areal *v);
    Vector3(const Vector3 &value);
    Vector3(const Vector4 &value);

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

#endif // VECTOR3_H
