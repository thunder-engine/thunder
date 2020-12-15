#include "math/amath.h"

/*!
    \class Vector3
    \brief The Vector3 class represents a vector or vertex in 3D space.
    \since Next 1.0
    \inmodule Math

    Vectors are one of the main building blocks of 3D representation and
    drawing. They consist of three coordinates, traditionally called
    x, y, and z.

    The Vector3 class can also be used to represent vertices in 3D space.
    We therefore do not need to provide a separate vertex class.

    \note By design values in the Vector3 instance are stored as \c float.
    This means that on platforms where the \c areal arguments to Vector3
    functions are represented by \c double values, it is possible to
    lose precision.

    \sa Vector2, Vector4, Quaternion
*/

/*!
    Constructs a null vector, i.e. with coordinates (0, 0, 0).
*/
Vector3::Vector3() :
    x(0),
    y(0),
    z(0) {
}
/*!
    Constructs a vector with coordinates (\a v).
*/
Vector3::Vector3(areal v) :
    x(v),
    y(v),
    z(v) {
}
/*!
    Constructs a vector with coordinates (\a x, \a y, \a z).
*/
Vector3::Vector3(areal x, areal y, areal z) :
    x(x),
    y(y),
    z(z) {
}
/*!
    Constructs a 3D vector from the specified 2D \a v. The z
    coordinate is set to \a z.

    \sa Vector2::Vector2()
*/
Vector3::Vector3(const Vector2 &v, areal z) :
    x(v.x),
    y(v.y),
    z(z) {
}
/*!
    Constructs a 3D vector from \a v (areal[3] array).
*/
Vector3::Vector3(const areal *v) :
    x(v[0]),
    y(v[1]),
    z(v[2]) {
}
/*!
    Returns true if this vector is equal to given \a vector; otherwise returns false.
    This operator uses an exact floating-point comparison.
*/
bool Vector3::operator==(const Vector3 &vector) const {
    return (x == vector.x) && (y == vector.y) && (z == vector.z);
}
/*!
    Returns true if this vector is NOT equal to given \a vector; otherwise returns false.
    This operator uses an exact floating-point comparison.
*/
bool Vector3::operator!=(const Vector3 &vector) const {
    return !(*this == vector);
}
/*!
    Returns true if this vector is bigger than given \a vector; otherwise returns false.
    This operator uses an exact floating-point comparison.
*/
bool Vector3::operator> (const Vector3 &vector) const {
    return (x > vector.x) && (y > vector.y) && (z > vector.z);
}
/*!
    Returns true if this vector is less than \a vector; otherwise returns false.
    This operator uses an exact floating-point comparison.
*/
bool Vector3::operator< (const Vector3 &vector) const {
    return (x < vector.x) && (y < vector.y) && (z < vector.z);
}
/*!
    Returns a copy of this vector, multiplied by the given \a factor.

    \sa operator*=()
*/
Vector3 Vector3::operator*(areal factor) const {
    return Vector3(x * factor, y * factor, z * factor);
}
/*!
    Returns a copy of this vector, multiplied by the given \a vector.

    \sa operator*=()
*/
Vector3 Vector3::operator*(const Vector3 &vector) const {
    return Vector3(x * vector.x, y * vector.y, z * vector.z);
}
/*!
    Returns a copy of this vector, divided by the given \a divisor.

    \sa operator/=()
*/
Vector3 Vector3::operator/(areal divisor) const {
    return Vector3(x / divisor, y / divisor, z / divisor);
}
/*!
    Returns a Vector3 object that is the sum of the this vector and \a vector; each component is added separately.

    \sa operator+=()
*/
Vector3 Vector3::operator+(const Vector3 &vector) const {
    return Vector3(x + vector.x, y + vector.y, z + vector.z);
}
/*!
    Returns a Vector3 object that is formed by changing the sign of
    all three components of the this vector.

    Equivalent to \c {Vector3(0,0,0) - vector}.
*/
Vector3 Vector3::operator-() const {
    return Vector3(-x, -y, -z);
}
/*!
    Returns a Vector3 object that is formed by subtracting \a vector from this vector;
    each component is subtracted separately.

    \sa operator-=()
*/
Vector3 Vector3::operator-(const Vector3 &vector) const {
    return Vector3(x - vector.x, y - vector.y, z - vector.z);
}
/*!
    Multiplies this vector's coordinates by the given \a factor, and
    returns a reference to this vector.

    \sa operator/=()
*/
Vector3 &Vector3::operator*=(areal factor) {
    return *this = *this * factor;
}
/*!
    Divides this vector's coordinates by the given \a divisor, and
    returns a reference to this vector.

    \sa operator*=()
*/
Vector3 &Vector3::operator/=(areal divisor) {
    return *this = *this / divisor;
}
/*!
    Adds the given \a vector to this vector and returns a reference to
    this vector.

    \sa operator-=()
*/
Vector3 &Vector3::operator+=(const Vector3 &vector) {
    return *this = *this + vector;
}
/*!
    Subtracts the given \a vector from this vector and returns a reference to
    this vector.

    \sa operator+=()
*/
Vector3 &Vector3::operator-=(const Vector3 &vector) {
    return *this = *this - vector;
}
/*!
    Returns the component of the vector at index position i as a modifiable reference.
    \a i must be a valid index position in the vector (i.e., 0 <= i < 3).
*/
areal &Vector3::operator[](int i) {
    return v[i];
}
/*!
    Returns the component of the vector at index position.
    \a i must be a valid index position in the vector (i.e., 0 <= i < 3).
*/
areal Vector3::operator[](int i) const {
    return v[i];
}
/*!
    Returns the length of this vector.

    \sa sqrLength()
*/
areal Vector3::length() const {
    return (areal)sqrt(sqrLength());
}
/*!
    Returns the squared length of this vector.

    \sa length()
*/
areal Vector3::sqrLength() const {
    return dot(*this);
}
/*!
    Normalizes the currect vector in place.
    Returns length of prenormalized vector.

    \sa length()
*/
areal Vector3::normalize() {
    areal len = length();
    if (len == 0.0f) {
        return 0.0f;
    }
    (*this) *= (1.0f / len);

    return len;
}
/*!
    Returns the cross-product of this vector and given \a vector.

    \sa dot()
*/
Vector3 Vector3::cross(const Vector3 &vector) const {
    return Vector3(y * vector.z - z * vector.y,
                   z * vector.x - x * vector.z,
                   x * vector.y - y * vector.x);
}
/*!
    Returns the dot-product of this vector and given \a vector.

    \sa cross()
*/
areal Vector3::dot(const Vector3 &vector) const {
    return x * vector.x + y * vector.y + z * vector.z;
}
/*!
    Returns the absplute value of this vector.
*/
Vector3 Vector3::abs() const {
    return Vector3(std::abs(x), std::abs(y), std::abs(z));
}
/*!
    Returns an absolute angle between current and provided \a vector.

    \sa signedAngle()
*/
areal Vector3::angle(const Vector3 &vector) const {
    return std::atan2(cross(vector).length(), dot(vector)) * RAD2DEG;
}
/*!
    Returns an signed angle between current and provided \a vector.
    The \a up vector around which the current and provided vectors are rotated.

    \sa angle()
*/
areal Vector3::signedAngle(const Vector3 &vector, const Vector3 up) const {
    Vector3 crossProduct = cross(vector);
    areal result = std::atan2(crossProduct.length(), dot(vector)) * RAD2DEG;
    if(crossProduct.dot(up) < 0) {
        result = -result;
    }
    return result;
}

