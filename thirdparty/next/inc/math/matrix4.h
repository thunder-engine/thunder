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

    Copyright: 2008-2025 Evgeniy Prikazchikov
*/

#ifndef MATRIX4_H
#define MATRIX4_H

#include <global.h>

class Vector3;
class Vector4;
class Matrix3;
class Quaternion;

class NEXT_LIBRARY_EXPORT Matrix4 {
public:
    Matrix4();
    Matrix4(const Matrix4 &matrix);
    Matrix4(const Matrix3 &matrix);
    Matrix4(const Vector3 &position, const Quaternion &rotation, const Vector3 &scale);

    Matrix4 &operator=(const Matrix4 &matrix);

    bool operator==(const Matrix4 &matrix) const;
    bool operator!=(const Matrix4 &matrix) const;

    Vector3 operator*(const Vector3 &vector) const;
    Vector4 operator*(const Vector4 &vector) const;
    Matrix4 operator*(areal factor) const;
    Matrix4 operator*(const Matrix4 &matrix) const;
    Matrix4 operator+(const Matrix4 &matrix) const;
    Matrix4 operator-(const Matrix4 &matrix) const;

    Matrix4 &operator*=(areal factor);
    Matrix4 &operator*=(const Matrix4 &matrix);
    Matrix4 &operator+=(const Matrix4 &matrix);
    Matrix4 &operator-=(const Matrix4 &matrix);

    areal &operator[](int i);
    areal operator[](int i) const;

    Matrix3 rotation() const;
    Matrix4 transpose() const;
    areal determinant() const;
    Matrix4 inverse() const;
    void reflect(const Vector4 &plane);
    void direction(const Vector3 &direction, const Vector3 &up);

    Vector3 euler();
    Vector3 position() const;

    void zero();
    void identity();
    void rotate(const Vector3 &axis, areal angle);
    void rotate(const Vector3 &angles);
    void scale(const Vector3 &vector);
    void translate(const Vector3 &vector);

    static Matrix4 perspective(areal fov, areal aspect, areal znear, areal zfar);
    static Matrix4 ortho(areal left, areal right, areal bottom, areal top, areal znear, areal zfar);
    static Matrix4 lookAt(const Vector3 &eye, const Vector3 &target, const Vector3 &up);

public:
    areal mat[16];

};

#endif // MATRIX4_H
