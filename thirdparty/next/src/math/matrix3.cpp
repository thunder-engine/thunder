#include "math/amath.h"

/*!
    \class Matrix3
    \brief The Matrix3 class represents a 3x3 rotation matrix in 3D space.
    \since Next 1.0
    \inmodule Math

    Internally the data is stored as column-major format,
    so as to be optimal for passing to OpenGL functions, which expect \b column-major data.

    \sa Vector3, Vector4, Quaternion, Matrix4
*/
/*!
    Constructs a identity matrix.
*/
Matrix3::Matrix3() {
    identity();
}
/*!
    Returns the result of multiplying this matrix and the given 3D \a vector.
*/
Vector3 Matrix3::operator*(const Vector3 &vector) const {
    Vector3 ret;
    ret[0] = mat[0] * vector[0] + mat[3] * vector[1] + mat[6] * vector[2];
    ret[1] = mat[1] * vector[0] + mat[4] * vector[1] + mat[7] * vector[2];
    ret[2] = mat[2] * vector[0] + mat[5] * vector[1] + mat[8] * vector[2];
    return ret;
}
/*!
    Returns the result of multiplying this matrix and the given 4D \a vector.
*/
Vector4 Matrix3::operator*(const Vector4 &vector) const {
    return Vector4(*this * Vector3(vector.x, vector.y, vector.z), vector.w);
}
/*!
    Returns the result of multiplying this matrix and the given \a factor.
*/
Matrix3 Matrix3::operator*(areal factor) const {
    Matrix3 ret;
    ret[0] = mat[0] * factor; ret[3] = mat[3] * factor; ret[6] = mat[6] * factor;
    ret[1] = mat[1] * factor; ret[4] = mat[4] * factor; ret[7] = mat[7] * factor;
    ret[2] = mat[2] * factor; ret[5] = mat[5] * factor; ret[8] = mat[8] * factor;
    return ret;
}
/*!
    Returns the result of multiplying this matrix by the given \a matrix.

    Note that matrix multiplication is not commutative, i.e. a*b != b*a.
*/
Matrix3 Matrix3::operator*(const Matrix3 &matrix) const {
    Matrix3 ret;
    ret[0] = mat[0] * matrix[0] + mat[3] * matrix[1] + mat[6] * matrix[2];
    ret[1] = mat[1] * matrix[0] + mat[4] * matrix[1] + mat[7] * matrix[2];
    ret[2] = mat[2] * matrix[0] + mat[5] * matrix[1] + mat[8] * matrix[2];
    ret[3] = mat[0] * matrix[3] + mat[3] * matrix[4] + mat[6] * matrix[5];
    ret[4] = mat[1] * matrix[3] + mat[4] * matrix[4] + mat[7] * matrix[5];
    ret[5] = mat[2] * matrix[3] + mat[5] * matrix[4] + mat[8] * matrix[5];
    ret[6] = mat[0] * matrix[6] + mat[3] * matrix[7] + mat[6] * matrix[8];
    ret[7] = mat[1] * matrix[6] + mat[4] * matrix[7] + mat[7] * matrix[8];
    ret[8] = mat[2] * matrix[6] + mat[5] * matrix[7] + mat[8] * matrix[8];
    return ret;
}
/*!
    Returns the sum of this matrix and the given \a matrix.
*/
Matrix3 Matrix3::operator+(const Matrix3 &matrix) const {
    Matrix3 ret;
    ret[0] = mat[0] + matrix[0]; ret[3] = mat[3] + matrix[3]; ret[6] = mat[6] + matrix[6];
    ret[1] = mat[1] + matrix[1]; ret[4] = mat[4] + matrix[4]; ret[7] = mat[7] + matrix[7];
    ret[2] = mat[2] + matrix[2]; ret[5] = mat[5] + matrix[5]; ret[8] = mat[8] + matrix[8];
    return ret;
}
/*!
    Returns the difference of this matrix and the given \a matrix.
*/
Matrix3 Matrix3::operator-(const Matrix3 &matrix) const {
    Matrix3 ret;
    ret[0] = mat[0] - matrix[0]; ret[3] = mat[3] - matrix[3]; ret[6] = mat[6] - matrix[6];
    ret[1] = mat[1] - matrix[1]; ret[4] = mat[4] - matrix[4]; ret[7] = mat[7] - matrix[7];
    ret[2] = mat[2] - matrix[2]; ret[5] = mat[5] - matrix[5]; ret[8] = mat[8] - matrix[8];
    return ret;
}
/*!
    Multiplies all elements of this matrix by \a factor.
*/
Matrix3 &Matrix3::operator*=(areal factor) {
    return *this = *this * factor;
}
/*!
    Returns the result of multiplying this matrix by the given \a matrix.
*/
Matrix3 &Matrix3::operator*=(const Matrix3 &matrix) {
    return *this = *this * matrix;
}
/*!
    Adds the contents of \a matrix to this matrix.
*/
Matrix3 &Matrix3::operator+=(const Matrix3 &matrix) {
    return *this = *this + matrix;
}
/*!
    Subtracts the contents of \a matrix from this matrix.
*/
Matrix3 &Matrix3::operator-=(const Matrix3 &matrix) {
    return *this = *this - matrix;
}
/*!
    Returns true if this matrix is equal to given \a matrix; otherwise returns false.
    This operator uses an exact floating-point comparison.
*/
bool Matrix3::operator==(const Matrix3 &matrix) const {
    for(int i = 0; i < 9; i++) {
        if(mat[i] != matrix.mat[i]) {
            return false;
        }
    }
    return true;
}
/*!
    Returns true if this matrix is NOT equal to given \a matrix; otherwise returns false.
    This operator uses an exact floating-point comparison.
*/
bool Matrix3::operator!=(const Matrix3 &matrix) const {
    return !(*this == matrix);
}
/*!
    Returns the component of the matrix at index position i as a modifiable reference.
    \a i must be a valid index position in the matrix (i.e., 0 <= i < 9).
    Data is stored as column-major format so this function retrieving data from rows in colmns.
*/
areal &Matrix3::operator[](int i) {
    return mat[i];
}
/*!
    Returns the component of the matrix at index position.
    \a i must be a valid index position in the matrix (i.e., 0 <= i < 9).
    Data is stored as column-major format so this function retrieving data from rows in colmns.
*/
areal Matrix3::operator[](int i) const {
    return mat[i];
}
/*!
    Returns this matrix, transposed about its diagonal.
*/
Matrix3 Matrix3::transpose() const {
    Matrix3 ret;
    ret[0] = mat[0]; ret[3] = mat[1]; ret[6] = mat[2];
    ret[1] = mat[3]; ret[4] = mat[4]; ret[7] = mat[5];
    ret[2] = mat[6]; ret[5] = mat[7]; ret[8] = mat[8];
    return ret;
}
/*!
    Returns the matrix determinant.
*/
areal Matrix3::determinant() const {
    areal det;
    det = mat[0] * mat[4] * mat[8];
    det += mat[3] * mat[7] * mat[2];
    det += mat[6] * mat[1] * mat[5];
    det -= mat[6] * mat[4] * mat[2];
    det -= mat[3] * mat[1] * mat[8];
    det -= mat[0] * mat[7] * mat[5];
    return det;
}
/*!
    Returns an inverted copy of this matrix.
*/
Matrix3 Matrix3::inverse() const {
    Matrix3 ret;
    areal idet = 1.0f / determinant();
    ret[0] =  (mat[4] * mat[8] - mat[7] * mat[5]) * idet;
    ret[1] = -(mat[1] * mat[8] - mat[7] * mat[2]) * idet;
    ret[2] =  (mat[1] * mat[5] - mat[4] * mat[2]) * idet;
    ret[3] = -(mat[3] * mat[8] - mat[6] * mat[5]) * idet;
    ret[4] =  (mat[0] * mat[8] - mat[6] * mat[2]) * idet;
    ret[5] = -(mat[0] * mat[5] - mat[3] * mat[2]) * idet;
    ret[6] =  (mat[3] * mat[7] - mat[6] * mat[4]) * idet;
    ret[7] = -(mat[0] * mat[7] - mat[6] * mat[1]) * idet;
    ret[8] =  (mat[0] * mat[4] - mat[3] * mat[1]) * idet;
    return ret;
}
/*!
    Clear this matrix, with 0.0 value for all components.
*/
void Matrix3::zero() {
    mat[0] = 0.0; mat[3] = 0.0; mat[6] = 0.0;
    mat[1] = 0.0; mat[4] = 0.0; mat[7] = 0.0;
    mat[2] = 0.0; mat[5] = 0.0; mat[8] = 0.0;
}
/*!
    Resets this matrix to an identity matrix.
*/
void Matrix3::identity() {
    mat[0] = 1.0; mat[3] = 0.0; mat[6] = 0.0;
    mat[1] = 0.0; mat[4] = 1.0; mat[7] = 0.0;
    mat[2] = 0.0; mat[5] = 0.0; mat[8] = 1.0;
}
/*!
    Rotate this matrix around \a axis to \a angle in rotation degrees.
*/
void Matrix3::rotate(const Vector3 &axis, areal angle) {
    areal rad = angle * DEG2RAD;
    areal c = (rad == PI * 0.5) ? 0 : (areal)cos(rad);
    areal s = (rad == PI) ? 0 : (areal)sin(rad);
    Vector3 v = axis;
    v.normalize();
    areal xy = v.x * v.y;
    areal yz = v.y * v.z;
    areal zx = v.z * v.x;
    areal xs = v.x * s;
    areal ys = v.y * s;
    areal zs = v.z * s;
    mat[0] = (1.0f - c) * v.x * v.x + c;
    mat[3] = (1.0f - c) * xy - zs;
    mat[6] = (1.0f - c) * zx + ys;

    mat[1] = (1.0f - c) * xy + zs;
    mat[4] = (1.0f - c) * v.y * v.y + c;
    mat[7] = (1.0f - c) * yz - xs;

    mat[2] = (1.0f - c) * zx - ys;
    mat[5] = (1.0f - c) * yz + xs;
    mat[8] = (1.0f - c) * v.z * v.z + c;
}
/*!
    Rotate this matrix with Euler \a angles represented by Vector3(pitch, yaw, roll) in rotation degrees.
*/
void Matrix3::rotate(const Vector3 &angles) {
    Matrix3 m;
    m.rotate(Vector3(1.0f, 0.0f, 0.0f), angles.x);
    *this   *= m;
    m.rotate(Vector3(0.0f, 1.0f, 0.0f), angles.y);
    *this   *= m;
    m.rotate(Vector3(0.0f, 0.0f, 1.0f), angles.z);
    *this   *= m;
}
/*!
    Scales the coordinate system by \a vector.
*/
void Matrix3::scale(const Vector3 &vector) {
    mat[0] = vector.x;  mat[3] = 0.0;       mat[6] = 0.0;
    mat[1] = 0.0;       mat[4] = vector.y;  mat[7] = 0.0;
    mat[2] = 0.0;       mat[5] = 0.0;       mat[8] = vector.z;
}
/*!
    Orthonormalize this matrix.
*/
void Matrix3::orthonormalize() {
    Vector3 x(mat[0], mat[1], mat[2]);
    Vector3 y(mat[3], mat[4], mat[5]);
    Vector3 z;
    x.normalize();
    z   = x.cross(y);
    z.normalize();
    y   = z.cross(x);
    y.normalize();
    mat[0] = x.x; mat[3] = y.x; mat[6] = z.x;
    mat[1] = x.y; mat[4] = y.y; mat[7] = z.y;
    mat[2] = x.z; mat[5] = y.z; mat[8] = z.z;
}
/*!
    Returns an Euler angles represented by Vector3(pitch, yaw, roll) in rotation degrees.
*/
Vector3 Matrix3::euler() {
    return Vector3(RAD2DEG * atan2(-mat[7], mat[8]),
                   RAD2DEG * atan2( mat[6], sqrt(mat[7] * mat[7] + mat[8] * mat[8])),
                   RAD2DEG * atan2(-mat[3], mat[0]));
}
