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
    Vector4(const Vector2 &vector);
    Vector4(const Vector2 &vector, areal z, areal w);
    Vector4(const Vector3 &vector);
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

#endif // VECTOR4_H
