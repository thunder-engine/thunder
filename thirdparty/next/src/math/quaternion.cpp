#include "math/amath.h"

/*!
    \class Quaternion
    \brief The Quaternion class represents rotations in 3D space.
    \since Next 1.0
    \inmodule Math

    Quaternions consist of a 3D rotation axis specified by the \c x, \c y, and \c z coordinates,
    and a \c w representing the rotation angle.

    \sa Vector2, Vector3, Vector4, Matrix3
*/

/*!
    Constructs an identity quaternion.
*/
Quaternion::Quaternion() :
    x(0),
    y(0),
    z(0),
    w(1) {
}
/*!
    Constructs a quaternion with values (\a x, \a y, \a z).
*/
Quaternion::Quaternion(areal x, areal y, areal z, areal w) :
    x(x),
    y(y),
    z(z),
    w(w) {

}
/*!
    Constructs a quaternion with rotation \a axis and \a angle in rotation degrees.
*/
Quaternion::Quaternion(const Vector3 &axis, areal angle) {
    areal length = axis.length();
    if(length != 0.0f) {
        length = 1.0f / length;
        areal sinangle = sin(angle * DEG2RAD / 2.0f);
        x = axis[0] * length * sinangle;
        y = axis[1] * length * sinangle;
        z = axis[2] * length * sinangle;
        w = cos(angle * DEG2RAD / 2.0f);
    } else {
        x = y = z = 0.0;
        w = 1.0;
    }
}
/*!
    Constructs a quaternion by Euler angles represented by Vector3(pitch, yaw, roll) \a euler in rotation degrees.
*/
Quaternion::Quaternion(const Vector3 &euler) {
    Vector3 rad2(euler.x * DEG2RAD * 0.5f,
                 euler.y * DEG2RAD * 0.5f,
                 euler.z * DEG2RAD * 0.5f);

    Vector3 c((rad2.x == PI * 0.5f) ? 0.0f : cos(rad2.x),
              (rad2.y == PI * 0.5f) ? 0.0f : cos(rad2.y),
              (rad2.z == PI * 0.5f) ? 0.0f : cos(rad2.z));
    Vector3 s((rad2.x == PI) ? 0.0f : sin(rad2.x),
              (rad2.y == PI) ? 0.0f : sin(rad2.y),
              (rad2.z == PI) ? 0.0f : sin(rad2.z));

    w = c.x * c.y * c.z + s.x * s.y * s.z;
    x = s.x * c.y * c.z - c.x * s.y * s.z;
    y = c.x * s.y * c.z + s.x * c.y * s.z;
    z = c.x * c.y * s.z - s.x * s.y * c.z;
}
/*!
    Constructs a quaternion by rotation matrix represented by Matrix3 \a matrix.
*/
Quaternion::Quaternion(const Matrix3 &matrix) {
    areal W = matrix[0] + matrix[4] + matrix[8];
    areal X = matrix[0] - matrix[4] - matrix[8];
    areal Y = matrix[4] - matrix[0] - matrix[8];
    areal Z = matrix[8] - matrix[0] - matrix[4];

    int index = 0;
    areal four = W;
    if(X > four) {
        four    = X;
        index   = 1;
    }
    if(Y > four) {
        four    = Y;
        index   = 2;
    }
    if(Z > four) {
        four    = Z;
        index   = 3;
    }

    areal biggest   = sqrt(four + 1) * 0.5f;
    areal mult = 0.25f / biggest;

    switch(index) {
        case 0: {
            w = biggest;
            x = (matrix[5] - matrix[7]) * mult; // m[1][2] - m[2][1]
            y = (matrix[6] - matrix[2]) * mult; // m[2][0] - m[0][2]
            z = (matrix[1] - matrix[3]) * mult; // m[0][1] - m[1][0]
        } break;
        case 1: {
            w = (matrix[5] - matrix[7]) * mult; // m[1][2] - m[2][1]
            x = biggest;
            y = (matrix[1] + matrix[3]) * mult; // m[0][1] + m[1][0]
            z = (matrix[6] + matrix[2]) * mult; // m[2][0] + m[0][2]
        } break;
        case 2: {
            w = (matrix[6] - matrix[2]) * mult; // m[2][0] - m[0][2]
            x = (matrix[1] + matrix[3]) * mult; // m[0][1] + m[1][0]
            y = biggest;
            z = (matrix[5] + matrix[7]) * mult; // m[1][2] + m[2][1]
        } break;
        case 3: {
            w = (matrix[1] - matrix[3]) * mult; // m[0][1] - m[1][0]
            x = (matrix[6] + matrix[2]) * mult; // m[2][0] + m[0][2]
            y = (matrix[5] + matrix[7]) * mult; // m[1][2] + m[2][1]
            z = biggest;
        } break;
        default: break;
    }
}
/*!
    Returns true if this quaternion is equal to given \a quaternion; otherwise returns false.
    This operator uses an exact floating-point comparison.
*/
bool Quaternion::operator==(const Quaternion &quaternion) const {
    return (x == quaternion.x) && (y == quaternion.y) && (z == quaternion.z) && (w == quaternion.w);
}
/*!
    Returns true if this quaternion is NOT equal to given \a quaternion; otherwise returns false.
    This operator uses an exact floating-point comparison.
*/
bool Quaternion::operator!=(const Quaternion &quaternion) const {
    return (x != quaternion.x) || (y != quaternion.y) || (z != quaternion.z) || (w != quaternion.w);
}
/*!
    Returns the component of the quaternion at index position i as a modifiable reference.
    \a i must be a valid index position in the quaternion (i.e., 0 <= i < 4).
*/
areal &Quaternion::operator[](int i) {
    return q[i];
}
/*!
    Returns the component of the quaternion at index position.
    \a i must be a valid index position in the quaternion (i.e., 0 <= i < 4).
*/
areal Quaternion::operator[](int i) const {
    return q[i];
}
/*!
    Multiplies this quaternion's coordinates by the given \a factor, and
    returns a reference to this quaternion.

    \sa operator/=()
*/
Quaternion &Quaternion::operator*=(areal factor) {
    return *this = *this * factor;
}
/*!
    Divides this quaternion's coordinates by the given \a divisor, and
    returns a reference to this quaternion.

    \sa operator*=()
*/
Quaternion &Quaternion::operator/=(areal divisor) {
    return *this = *this / divisor;
}
/*!
    Returns a copy of this quaternion, multiplied by the given \a factor.

    \sa operator*=()
*/
Quaternion Quaternion::operator*(areal factor) const {
    return Quaternion(x * factor, y * factor, z * factor, w * factor);
}
/*!
    Multiplies this quaternion and \a quaternion using quaternion multiplication.
    The result corresponds to applying both of the rotations specified by this quaternion and \a quaternion.
*/
Quaternion Quaternion::operator*(const Quaternion &quaternion) const {
    Quaternion ret;
    ret.x = w * quaternion.x + x * quaternion.x + y * quaternion.z - z * quaternion.y;
    ret.y = w * quaternion.y + y * quaternion.w + z * quaternion.x - x * quaternion.z;
    ret.z = w * quaternion.z + z * quaternion.w + x * quaternion.y - y * quaternion.x;
    ret.w = w * quaternion.w - x * quaternion.x - y * quaternion.y - z * quaternion.z;
    return ret;
}
/*!
    Rotates a \a vector vec with this quaternion to produce a new vector in 3D space.
*/
Vector3 Quaternion::operator*(const Vector3 &vector) const {
    Vector3 vec(x, y, z);

    Vector3 uv    = vec.cross(vector);
    Vector3 uuv   = vec.cross(uv);

    return vector + ((uv * w) + uuv) * 2;
}
/*!
    Returns a copy of this quaternion, divided by the given \a divisor.

    \sa operator/=()
*/
Quaternion Quaternion::operator/(areal divisor) const {
    return Quaternion(x / divisor, y / divisor, z / divisor, w / divisor);
}
/*!
    Returns the length of this quaternion.

    \sa sqrLength()
*/
areal Quaternion::length() const {
     return (areal)sqrt(sqrLength());
}
/*!
    Returns the squared length of this quaternion.

    \sa length()
*/
areal Quaternion::sqrLength() const {
    return x * x + y * y + z * z + w * w;
}
/*!
    Normalizes the currect quaternion in place.
    Returns length of prenormalized quaternion.

    \sa length()
*/
areal Quaternion::normalize() {
    areal len = length();
    if (len == 0.0f)
        return 0.0f;
    (*this) /= len;

    return len;
}
/*!
    Returns the dot-product of this quaternion and given \a quaternion.
*/
areal Quaternion::dot(const Quaternion &quaternion) const {
    return (x * quaternion.x + y * quaternion.y + z * quaternion.z + w * quaternion.w);
}
/*!
    Returns the inverse of this quaternion.
*/
Quaternion Quaternion::inverse() const {
    Quaternion ret;
    ret.w = w; ret.x =-x; ret.y =-y; ret.z =-z;
    return ret;
}
/*!
    Returns true if \a quaternion approximately equal.
*/
bool Quaternion::equal(const Quaternion &quaternion) const {
    return abs(x - quaternion.x) <= FLT_EPSILON &&
           abs(y - quaternion.y) <= FLT_EPSILON &&
           abs(z - quaternion.z) <= FLT_EPSILON &&
           abs(w - quaternion.w) <= FLT_EPSILON;
}
/*!
    Linear inerpolation between \a q0 and \a q1 with \a t factor.
*/
void Quaternion::mix(const Quaternion &q0, const Quaternion &q1, areal t) {
    areal k0, k1, cosomega = q0.dot(q1);
    Quaternion q;
    if(cosomega < 0.0f) {
        cosomega = -cosomega;
        q.x = -q1.x;
        q.y = -q1.y;
        q.z = -q1.z;
        q.w = -q1.w;
    } else {
        q.x = q1.x;
        q.y = q1.y;
        q.z = q1.z;
        q.w = q1.w;
    }
    if(1.0f - cosomega > 1e-6f) {
        areal omega = acos(cosomega);
        areal sinomega = sin(omega);
        k0 = sin((1.0f - t) * omega) / sinomega;
        k1 = sin(t * omega) / sinomega;
    } else {
        k0 = 1.0f - t;
        k1 = t;
    }
    x = q0.x * k0 + q.x * k1;
    y = q0.y * k0 + q.y * k1;
    z = q0.z * k0 + q.z * k1;
    w = q0.w * k0 + q.w * k1;
}
/*!
    Returns the rotation matrix for this quaternion.
*/
Matrix3 Quaternion::toMatrix() const {
    Matrix3 ret;
    areal qx(x * 2.0f);
    areal qy(y * 2.0f);
    areal qz(z * 2.0f);
    areal qxx(x * qx);
    areal qyy(y * qy);
    areal qzz(z * qz);
    areal qxz(x * qz);
    areal qxy(x * qy);
    areal qyz(y * qz);
    areal qwx(w * qx);
    areal qwy(w * qy);
    areal qwz(w * qz);

    ret[0] = 1 - (qyy + qzz);
    ret[1] = qxy + qwz;
    ret[2] = qxz - qwy;

    ret[3] = qxy - qwz;
    ret[4] = 1 - (qxx + qzz);
    ret[5] = qyz + qwx;

    ret[6] = qxz + qwy;
    ret[7] = qyz - qwx;
    ret[8] = 1 - (qxx + qyy);
    return ret;
}
/*!
    Returns the Euler angles represented by Vector3(pitch, yaw, roll) in rotation degrees.
*/
Vector3 Quaternion::euler() const {
    return toMatrix().euler();
}
/*!
    Retrives a quaternion as rotation \a axis and \a angle in rotation degrees.
*/
void Quaternion::axisAngle(Vector3 &axis, areal &angle) {
    angle = 2.0f * acos(w);
    areal r = (1.0f) / sqrt(1.0f - w * w);
    axis.x = x * r;
    axis.y = y * r;
    axis.z = z * r;
}
