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
    Constructs a quaternion with rotation axis \a dir and \a angle in rotation degrees.
*/
Quaternion::Quaternion(const Vector3 &dir, areal angle) {
    areal length = dir.length();
    if(length != 0.0) {
        length = 1.0f / length;
        areal sinangle = (areal)sin(angle * DEG2RAD / 2.0f);
        x = dir[0] * length * sinangle;
        y = dir[1] * length * sinangle;
        z = dir[2] * length * sinangle;
        w = (areal)cos(angle * DEG2RAD / 2.0f);
    } else {
        x = y = z = 0.0;
        w = 1.0;
    }
}
/*!
    Constructs a quaternion by Euler angles represented by Vector3(pitch, yaw, roll) \a euler in rotation degrees.
*/
Quaternion::Quaternion(const Vector3 &euler) {
    Vector3 rad2(euler.x * DEG2RAD * 0.5f, euler.y * DEG2RAD * 0.5f, euler.z * DEG2RAD * 0.5f);
    Vector3 c(cos(rad2.x), cos(rad2.y), cos(rad2.z));
    Vector3 s(sin(rad2.x), sin(rad2.y), sin(rad2.z));

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

    areal biggest   = sqrt(four + 1) * 0.5;
    areal mult = 0.25 / biggest;

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
const areal Quaternion::operator[](int i) const {
    return q[i];
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
    Returns the inverse of this quaternion.
*/
Quaternion Quaternion::inverse() const {
    Quaternion ret;
    ret.w = w; ret.x =-x; ret.y =-y; ret.z =-z;
    return ret;
}
/*!
    Linear inerpolation between \a q0 and \a q1 with \a t factor.
*/
void Quaternion::mix(const Quaternion &q0, const Quaternion &q1, areal t) {
    areal k0,k1,cosomega = q0.x * q1.x + q0.y * q1.y + q0.z * q1.z + q0.w * q1.w;
    Quaternion q;
    if(cosomega < 0.0) {
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
    if(1.0 - cosomega > 1e-6) {
        areal omega = (areal)acos(cosomega);
        areal sinomega = (areal)sin(omega);
        k0 = (areal)sin((1.0f - t) * omega) / sinomega;
        k1 = (areal)sin(t * omega) / sinomega;
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
    areal qxx(x * x);
    areal qyy(y * y);
    areal qzz(z * z);
    areal qxz(x * z);
    areal qxy(x * y);
    areal qyz(y * z);
    areal qwx(w * x);
    areal qwy(w * y);
    areal qwz(w * z);

    ret[0] = 1 - 2 * (qyy +  qzz);
    ret[1] = 2 * (qxy + qwz);
    ret[2] = 2 * (qxz - qwy);

    ret[3] = 2 * (qxy - qwz);
    ret[4] = 1 - 2 * (qxx +  qzz);
    ret[5] = 2 * (qyz + qwx);

    ret[6] = 2 * (qxz + qwy);
    ret[7] = 2 * (qyz - qwx);
    ret[8] = 1 - 2 * (qxx +  qyy);
    return ret;
}
/*!
    Returns the Euler angles represented by Vector3(pitch, yaw, roll) in rotation degrees.
*/
Vector3 Quaternion::euler() const {
    return toMatrix().euler();
}
