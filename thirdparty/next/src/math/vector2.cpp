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

#include "math/amath.h"

/*!
    \class Vector2
    \brief The Vector2 class represents a vector or vertex in 2D space.
    \since Next 1.0
    \inmodule Math

    Vectors are one of the main building blocks of 2D representation and
    drawing. They consist of three coordinates, traditionally called
    x and y.

    The Vector2 class can also be used to represent vertices in 2D space.
    We therefore do not need to provide a separate vertex class.

    \note By design values in the Vector2 instance are stored as \c float.
    This means that on platforms where the \c areal arguments to Vector2
    functions are represented by \c double values, it is possible to
    lose precision.

    \sa Vector3, Vector4, Quaternion
*/

/*!
    Constructs a null vector, i.e. with coordinates (0, 0).
*/
Vector2::Vector2() :
    x(0),
    y(0) {
}
/*!
    Constructs a vector with coordinates (\a v).
*/
Vector2::Vector2(areal v) :
    x(v),
    y(v) {
}
/*!
    Constructs a vector with coordinates (\a x, \a y).
*/
Vector2::Vector2(areal x, areal y) :
    x(x),
    y(y) {
}
/*!
    Copy constructor.
*/
Vector2::Vector2(const Vector2 &vector) :
    x(vector.x),
    y(vector.y) {
}

Vector2::Vector2(const Vector3 &vector) :
    x(vector.x),
    y(vector.y) {
}

Vector2::Vector2(const Vector4 &vector) :
    x(vector.x),
    y(vector.y) {
}

/*!
    Assignment operator.
    The \a value will be assigned to this object.
*/
Vector2 &Vector2::operator=(const Vector2 &value) {
    x = value.x;
    y = value.y;

    return *this;
}
/*!
    Returns true if this vector is equal to given \a vector; otherwise returns false.
    This operator uses an exact floating-point comparison.
*/
bool Vector2::operator==(const Vector2 &vector) const {
    return (x == vector.x) && (y == vector.y);
}
/*!
    Returns true if this vector is NOT equal to given \a vector; otherwise returns false.
    This operator uses an exact floating-point comparison.
*/
bool Vector2::operator!=(const Vector2 &vector) const {
    return !(*this == vector);
}
/*!
    Returns true if this vector is bigger than given \a vector; otherwise returns false.
    This operator uses an exact floating-point comparison.
*/
bool Vector2::operator>(const Vector2 &vector) const {
    return (x > vector.x) && (y > vector.y);
}
/*!
    Returns true if this vector is less than \a vector; otherwise returns false.
    This operator uses an exact floating-point comparison.
*/
bool Vector2::operator<(const Vector2 &vector) const {
    return (x < vector.x) && (y < vector.y);
}
/*!
    Returns a copy of this vector, multiplied by the given \a factor.

    \sa operator*=()
*/
Vector2 Vector2::operator*(areal factor) const {
    return Vector2(x * factor, y * factor);
}
/*!
    Returns a copy of this vector, multiplied by the given \a vector.

    \sa operator*=()
*/
Vector2 Vector2::operator*(const Vector2 &vector) const {
    return Vector2(x * vector.x, y * vector.y);
}
/*!
    Returns a copy of this vector, divided by the given \a divisor.

    \sa operator/=()
*/
Vector2 Vector2::operator/(areal divisor) const {
    return Vector2(x / divisor, y / divisor);
}
/*!
    Returns a Vector2 object that is the sum of the this vector and \a vector; each component is added separately.

    \sa operator+=()
*/
Vector2 Vector2::operator+(const Vector2 &vector) const {
    return Vector2(x + vector.x, y + vector.y);
}
/*!
    Returns a Vector2 object that is formed by changing the sign of
    all three components of the this vector.

    Equivalent to \c {Vector2(0,0) - vector}.
*/
Vector2 Vector2::operator-() const {
    return Vector2(-x, -y);
}
/*!
    Returns a Vector2 object that is formed by subtracting \a vector from this vector;
    each component is subtracted separately.

    \sa operator-=()
*/
Vector2 Vector2::operator-(const Vector2 &vector) const {
    return Vector2(x - vector.x, y - vector.y);
}
/*!
    Multiplies this vector's coordinates by the given \a factor, and
    returns a reference to this vector.

    \sa operator/=()
*/
Vector2 &Vector2::operator*=(areal factor) {
    return *this = *this * factor;
}
/*!
    Divides this vector's coordinates by the given \a divisor, and
    returns a reference to this vector.

    \sa operator*=()
*/
Vector2 &Vector2::operator/=(areal divisor) {
    return *this = *this / divisor;
}
/*!
    Adds the given \a vector to this vector and returns a reference to
    this vector.

    \sa operator-=()
*/
Vector2 &Vector2::operator+=(const Vector2 &vector) {
    return *this = *this + vector;
}
/*!
    Subtracts the given \a vector from this vector and returns a reference to
    this vector.

    \sa operator+=()
*/
Vector2 &Vector2::operator-=(const Vector2 &vector) {
    return *this = *this - vector;
}
/*!
    Returns the component of the vector at index position i as a modifiable reference.
    \a i must be a valid index position in the vector (i.e., 0 <= i < 2).
*/
areal &Vector2::operator[](int i) {
    return v[i];
}
/*!
    Returns the component of the vector at index position.
    \a i must be a valid index position in the vector (i.e., 0 <= i < 2).
*/
areal Vector2::operator[](int i) const {
    return v[i];
}
/*!
    Returns the length of this vector.

    \sa sqrLength()
*/
areal Vector2::length() const {
    return (areal)sqrt(sqrLength());
}
/*!
    Returns the squared length of this vector.

    \sa length()
*/
areal Vector2::sqrLength() const {
    return x * x + y * y;
}
/*!
    Normalizes the currect vector in place.
    Returns length of prenormalized vector.

    \sa length()
*/
areal Vector2::normalize() {
    areal len = length();
    if(len == 0.0f) {
        return 0.0f;
    }
    (*this) *= (1.0f / len);

    return len;
}
/*!
    Returns the cross-product of this vector and given \a vector.

    \sa dot()
*/
areal Vector2::cross(const Vector2 &vector) const {
    return x * vector.y - y * vector.x;
}
/*!
    Returns the dot-product of this vector and given \a vector.

    \sa cross()
*/
areal Vector2::dot(const Vector2 &vector) const {
    return x * vector.x + y * vector.y;
}
