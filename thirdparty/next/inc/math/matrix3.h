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

#ifndef MATRIX3_H
#define MATRIX3_H

#include <global.h>

class Vector3;
class Vector4;

class NEXT_LIBRARY_EXPORT Matrix3 {
public:
    Matrix3();

    Matrix3 &operator=(const Matrix3 &value);

    bool operator==(const Matrix3 &matrix) const;
    bool operator!=(const Matrix3 &matrix) const;

    Vector3 operator*(const Vector3 &vector) const;
    Vector4 operator*(const Vector4 &vector) const;
    Matrix3 operator*(areal factor) const;
    Matrix3 operator*(const Matrix3 &matrix) const;
    Matrix3 operator+(const Matrix3 &matrix) const;
    Matrix3 operator-(const Matrix3 &matrix) const;

    Matrix3 &operator*=(areal factor);
    Matrix3 &operator*=(const Matrix3 &matrix);
    Matrix3 &operator+=(const Matrix3 &matrix);
    Matrix3 &operator-=(const Matrix3 &matrix);

    areal &operator[](int i);
    areal operator[](int i) const;

    Matrix3 transpose() const;
    areal determinant() const;
    Matrix3 inverse() const;

    void zero();
    void identity();
    void rotate(const Vector3 &axis, areal angle);
    void rotate(const Vector3 &angles);
    void scale(const Vector3 &vector);

    void orthonormalize();

    Vector3 euler();

public:
    areal mat[9];

};

#endif // MATRIX3_H
