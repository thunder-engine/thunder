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

#endif // VECTOR2_H
