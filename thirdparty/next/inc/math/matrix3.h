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

#endif /* MATRIX3_H */
