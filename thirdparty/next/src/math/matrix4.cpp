#include "math/amath.h"

/*!
    \class Matrix4
    \brief The Matrix4 class represents a 4x4 transform matrix in 3D space.
    \since Next 1.0
    \inmodule Math

    Internally the data is stored as column-major format,
    so as to be optimal for passing to OpenGL functions, which expect \b column-major data.

    \sa Vector3, Vector4, Quaternion, Matrix3
*/
/*!
    Constructs an identity matrix.
*/
Matrix4::Matrix4() {
    identity();
}
/*!
    Constructs a transform matrix with rotation \a matrix.
*/
Matrix4::Matrix4(const Matrix3 &matrix) {
    mat[0] = matrix[0]; mat[4] = matrix[3]; mat[ 8] = matrix[6]; mat[12] = 0.0f;
    mat[1] = matrix[1]; mat[5] = matrix[4]; mat[ 9] = matrix[7]; mat[13] = 0.0f;
    mat[2] = matrix[2]; mat[6] = matrix[5]; mat[10] = matrix[8]; mat[14] = 0.0f;
    mat[3] = 0.0f;      mat[7] = 0.0f;      mat[11] = 0.0f;      mat[15] = 1.0f;
}
/*!
    Constructs matrix by given \a position, \a rotation and \a scale.
*/
Matrix4::Matrix4(const Vector3 &position, const Quaternion &rotation, const Vector3 &scale) {
    translate(position);
    *this *= Matrix4(rotation.toMatrix());

    Matrix4 m;
    m.scale(scale);
    *this *= m;
}
/*!
    Returns true if this matrix is equal to given \a matrix; otherwise returns false.
    This operator uses an exact floating-point comparison.
*/
bool Matrix4::operator==(const Matrix4 &matrix) const {
    for(int i = 0; i < 16; i++) {
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
bool Matrix4::operator!=(const Matrix4 &matrix) const {
    return !(*this == matrix);
}
/*!
    Returns the result of multiplying this matrix and the given 3D \a vector.
*/
Vector3 Matrix4::operator*(const Vector3 &vector) const {
    Vector3 ret;
    ret[0] = mat[0] * vector[0] + mat[4] * vector[1] + mat[ 8] * vector[2] + mat[12];
    ret[1] = mat[1] * vector[0] + mat[5] * vector[1] + mat[ 9] * vector[2] + mat[13];
    ret[2] = mat[2] * vector[0] + mat[6] * vector[1] + mat[10] * vector[2] + mat[14];

    return ret;
}
/*!
    Returns the result of multiplying this matrix and the given 4D \a vector.
*/
Vector4 Matrix4::operator*(const Vector4 &vector) const {
    Vector4 ret;
    ret[0] = mat[0] * vector[0] + mat[4] * vector[1] + mat[8]  * vector[2] + mat[12] * vector[3];
    ret[1] = mat[1] * vector[0] + mat[5] * vector[1] + mat[9]  * vector[2] + mat[13] * vector[3];
    ret[2] = mat[2] * vector[0] + mat[6] * vector[1] + mat[10] * vector[2] + mat[14] * vector[3];
    ret[3] = mat[3] * vector[0] + mat[7] * vector[1] + mat[11] * vector[2] + mat[15] * vector[3];

    return ret;
}
/*!
    Returns the result of multiplying this matrix and the given \a factor.
*/
Matrix4 Matrix4::operator*(areal factor) const {
    Matrix4 ret;
    ret[0] = mat[0] * factor; ret[4] = mat[4] * factor; ret[8]  = mat[8]  * factor; ret[12] = mat[12] * factor;
    ret[1] = mat[1] * factor; ret[5] = mat[5] * factor; ret[9]  = mat[9]  * factor; ret[13] = mat[13] * factor;
    ret[2] = mat[2] * factor; ret[6] = mat[6] * factor; ret[10] = mat[10] * factor; ret[14] = mat[14] * factor;
    ret[3] = mat[3] * factor; ret[7] = mat[7] * factor; ret[11] = mat[11] * factor; ret[15] = mat[15] * factor;

    return ret;
}
/*!
    Returns the result of multiplying this matrix by the given \a matrix.

    Note that matrix multiplication is not commutative, i.e. a*b != b*a.
*/
Matrix4 Matrix4::operator*(const Matrix4 &matrix) const {
    Matrix4 ret;
    ret[0]  = mat[0] * matrix[0]  + mat[4] * matrix[1]  + mat[8]  * matrix[2]  + mat[12] * matrix[3];
    ret[1]  = mat[1] * matrix[0]  + mat[5] * matrix[1]  + mat[9]  * matrix[2]  + mat[13] * matrix[3];
    ret[2]  = mat[2] * matrix[0]  + mat[6] * matrix[1]  + mat[10] * matrix[2]  + mat[14] * matrix[3];
    ret[3]  = mat[3] * matrix[0]  + mat[7] * matrix[1]  + mat[11] * matrix[2]  + mat[15] * matrix[3];
    ret[4]  = mat[0] * matrix[4]  + mat[4] * matrix[5]  + mat[8]  * matrix[6]  + mat[12] * matrix[7];
    ret[5]  = mat[1] * matrix[4]  + mat[5] * matrix[5]  + mat[9]  * matrix[6]  + mat[13] * matrix[7];
    ret[6]  = mat[2] * matrix[4]  + mat[6] * matrix[5]  + mat[10] * matrix[6]  + mat[14] * matrix[7];
    ret[7]  = mat[3] * matrix[4]  + mat[7] * matrix[5]  + mat[11] * matrix[6]  + mat[15] * matrix[7];
    ret[8]  = mat[0] * matrix[8]  + mat[4] * matrix[9]  + mat[8]  * matrix[10] + mat[12] * matrix[11];
    ret[9]  = mat[1] * matrix[8]  + mat[5] * matrix[9]  + mat[9]  * matrix[10] + mat[13] * matrix[11];
    ret[10] = mat[2] * matrix[8]  + mat[6] * matrix[9]  + mat[10] * matrix[10] + mat[14] * matrix[11];
    ret[11] = mat[3] * matrix[8]  + mat[7] * matrix[9]  + mat[11] * matrix[10] + mat[15] * matrix[11];
    ret[12] = mat[0] * matrix[12] + mat[4] * matrix[13] + mat[8]  * matrix[14] + mat[12] * matrix[15];
    ret[13] = mat[1] * matrix[12] + mat[5] * matrix[13] + mat[9]  * matrix[14] + mat[13] * matrix[15];
    ret[14] = mat[2] * matrix[12] + mat[6] * matrix[13] + mat[10] * matrix[14] + mat[14] * matrix[15];
    ret[15] = mat[3] * matrix[12] + mat[7] * matrix[13] + mat[11] * matrix[14] + mat[15] * matrix[15];

    return ret;
}
/*!
    Returns the sum of this matrix and the given \a matrix.
*/
Matrix4 Matrix4::operator+(const Matrix4 &matrix) const {
    Matrix4 ret;
    ret[0] = mat[0] + matrix[0]; ret[4] = mat[4] + matrix[4]; ret[8]  = mat[8]  + matrix[8];  ret[12] = mat[12] + matrix[12];
    ret[1] = mat[1] + matrix[1]; ret[5] = mat[5] + matrix[5]; ret[9]  = mat[9]  + matrix[9];  ret[13] = mat[13] + matrix[13];
    ret[2] = mat[2] + matrix[2]; ret[6] = mat[6] + matrix[6]; ret[10] = mat[10] + matrix[10]; ret[14] = mat[14] + matrix[14];
    ret[3] = mat[3] + matrix[3]; ret[7] = mat[7] + matrix[7]; ret[11] = mat[11] + matrix[11]; ret[15] = mat[15] + matrix[15];

    return ret;
}
/*!
    Returns the difference of this matrix and the given \a matrix.
*/
Matrix4 Matrix4::operator-(const Matrix4 &matrix) const {
    Matrix4 ret;
    ret[0] = mat[0] - matrix[0]; ret[4] = mat[4] - matrix[4]; ret[8]  = mat[8]  - matrix[8];  ret[12] = mat[12] - matrix[12];
    ret[1] = mat[1] - matrix[1]; ret[5] = mat[5] - matrix[5]; ret[9]  = mat[9]  - matrix[9];  ret[13] = mat[13] - matrix[13];
    ret[2] = mat[2] - matrix[2]; ret[6] = mat[6] - matrix[6]; ret[10] = mat[10] - matrix[10]; ret[14] = mat[14] - matrix[14];
    ret[3] = mat[3] - matrix[3]; ret[7] = mat[7] - matrix[7]; ret[11] = mat[11] - matrix[11]; ret[15] = mat[15] - matrix[15];

    return ret;
}
/*!
    Multiplies all elements of this matrix by \a factor.
*/
Matrix4 &Matrix4::operator*=(areal factor) {
    return *this = *this * factor;
}
/*!
    Returns the result of multiplying this matrix by the given \a matrix.
*/
Matrix4 &Matrix4::operator*=(const Matrix4 &matrix) {
    return *this = *this * matrix;
}
/*!
    Adds the contents of \a matrix to this matrix.
*/
Matrix4 &Matrix4::operator+=(const Matrix4 &matrix) {
    return *this = *this + matrix;
}
/*!
    Subtracts the contents of \a matrix from this matrix.
*/
Matrix4 &Matrix4::operator-=(const Matrix4 &matrix) {
    return *this = *this - matrix;
}
/*!
    Returns the component of the matrix at index position i as a modifiable reference.
    \a i must be a valid index position in the matrix (i.e., 0 <= i < 16).
    Data is stored as column-major format so this function retrieving data from rows in colmns.
*/
areal &Matrix4::operator[](int i) {
    return mat[i];
}
/*!
    Returns the component of the matrix at index position.
    \a i must be a valid index position in the matrix (i.e., 0 <= i < 16).
    Data is stored as column-major format so this function retrieving data from rows in colmns.
*/
areal Matrix4::operator[](int i) const {
    return mat[i];
}
/*!
    Returns rotation matrix from this matrix.
*/
Matrix3 Matrix4::rotation() const {
    Matrix3 ret;
    ret[0] = mat[0]; ret[3] = mat[4]; ret[6] = mat[ 8];
    ret[1] = mat[1]; ret[4] = mat[5]; ret[7] = mat[ 9];
    ret[2] = mat[2]; ret[5] = mat[6]; ret[8] = mat[10];

    return ret;
}
/*!
    Returns this matrix, transposed about its diagonal.
*/
Matrix4 Matrix4::transpose() const {
    Matrix4 ret;
    ret[0] = mat[0];  ret[4] = mat[1];  ret[8]  = mat[2];  ret[12] = mat[3];
    ret[1] = mat[4];  ret[5] = mat[5];  ret[9]  = mat[6];  ret[13] = mat[7];
    ret[2] = mat[8];  ret[6] = mat[9];  ret[10] = mat[10]; ret[14] = mat[11];
    ret[3] = mat[12]; ret[7] = mat[13]; ret[11] = mat[14]; ret[15] = mat[15];
    return ret;
}

inline Matrix3 subMatrix(const float mat[16], int x, int y) {
    Matrix3 ret;

    for(int dx = 0; dx < 3; dx++ ) {
        for(int dy = 0; dy < 3; dy++ ) {
            int sx = dx + ( ( dx >= x ) ? 1 : 0 );
            int sy = dy + ( ( dy >= y ) ? 1 : 0 );

            ret[dx * 3 + dy] = mat[sx * 4 + sy];
        }
    }

    return ret;
}
/*!
    Returns the matrix determinant.
*/
areal Matrix4::determinant() const {
    areal ret = mat[0] * subMatrix( mat, 0, 0 ).determinant();
    ret -= mat[1] * subMatrix( mat, 0, 1 ).determinant();
    ret += mat[2] * subMatrix( mat, 0, 2 ).determinant();
    ret -= mat[3] * subMatrix( mat, 0, 3 ).determinant();

    return ret;
}
/*!
    Returns an inverted copy of this matrix.
*/
Matrix4 Matrix4::inverse() const {
    areal det = determinant();
    if(det == 0.0f) {
        return Matrix4();
    }

    Matrix4 ret;
    for(int x = 0; x < 4; x++) {
        for(int y = 0; y < 4; y++) {
            int sign = 1 - ( (x + y) % 2 ) * 2;
            ret[x + y * 4] = ( subMatrix(mat, x, y ).determinant() * sign ) / det;
        }
    }
    return ret;
}
/*!
    Clear this matrix, with 0.0 value for all components.
*/
void Matrix4::zero() {
    mat[0] = 0.0f; mat[4] = 0.0f; mat[8 ] = 0.0f; mat[12] = 0.0f;
    mat[1] = 0.0f; mat[5] = 0.0f; mat[9 ] = 0.0f; mat[13] = 0.0f;
    mat[2] = 0.0f; mat[6] = 0.0f; mat[10] = 0.0f; mat[14] = 0.0f;
    mat[3] = 0.0f; mat[7] = 0.0f; mat[11] = 0.0f; mat[15] = 0.0f;
}
/*!
    Resets this matrix to an identity matrix.
*/
void Matrix4::identity() {
    mat[0] = 1.0f; mat[4] = 0.0f; mat[8 ] = 0.0f; mat[12] = 0.0f;
    mat[1] = 0.0f; mat[5] = 1.0f; mat[9 ] = 0.0f; mat[13] = 0.0f;
    mat[2] = 0.0f; mat[6] = 0.0f; mat[10] = 1.0f; mat[14] = 0.0f;
    mat[3] = 0.0f; mat[7] = 0.0f; mat[11] = 0.0f; mat[15] = 1.0f;
}
/*!
    Rotate this matrix around \a axis to \a angle in degrees.
*/
void Matrix4::rotate(const Vector3 &axis, areal angle) {
    Matrix3 m;
    m.rotate(axis, angle);
    *this = m;
}
/*!
    Rotate this matrix with Euler \a angles represented by Vector3(pitch, yaw, roll) in degrees.
*/
void Matrix4::rotate(const Vector3 &angles) {
    Matrix3 m;
    m.rotate(angles);
    *this = m;
}
/*!
    Scales the coordinate system by \a vector.
*/
void Matrix4::scale(const Vector3 &vector) {
    mat[0] = vector.x; mat[4] = 0.0f;     mat[8]  = 0.0f;     mat[12] = 0.0f;
    mat[1] = 0.0f;     mat[5] = vector.y; mat[9]  = 0.0f;     mat[13] = 0.0f;
    mat[2] = 0.0f;     mat[6] = 0.0f;     mat[10] = vector.z; mat[14] = 0.0f;
    mat[3] = 0.0f;     mat[7] = 0.0f;     mat[11] = 0.0f;     mat[15] = 1.0f;
}
/*!
    Move the coordinate system to \a vector.
*/
void Matrix4::translate(const Vector3 &vector) {
    mat[0] = 1.0f; mat[4] = 0.0f; mat[8]  = 0.0f; mat[12] = vector.x;
    mat[1] = 0.0f; mat[5] = 1.0f; mat[9]  = 0.0f; mat[13] = vector.y;
    mat[2] = 0.0f; mat[6] = 0.0f; mat[10] = 1.0f; mat[14] = vector.z;
    mat[3] = 0.0f; mat[7] = 0.0f; mat[11] = 0.0f; mat[15] = 1.0f;
}
/*!
    Returns an Euler angles represented by Vector3(pitch, yaw, roll) in rotation degrees.
*/
Vector3 Matrix4::euler() {
    return rotation().euler();
}
/*!
    Constructs a matrix that reflects the coordinate system about the \a plane.
*/
void Matrix4::reflect(const Vector4 &plane) {
    areal x = plane.x;
    areal y = plane.y;
    areal z = plane.z;
    areal x2 = x * 2.0f;
    areal y2 = y * 2.0f;
    areal z2 = z * 2.0f;

    mat[0] = 1.0f - x * x2; mat[4] = -y * x2;       mat[8] = -z * x2;        mat[12] = -plane.w * x2;
    mat[1] = -x * y2;       mat[5] = 1.0f - y * y2; mat[9] = -z * y2;        mat[13] = -plane.w * y2;
    mat[2] = -x * z2;       mat[6] = -y * z2;       mat[10] = 1.0f - z * z2; mat[14] = -plane.w * z2;
    mat[3] = 0.0f;          mat[7] = 0.0f;          mat[11] = 0.0f;          mat[15] = 1.0f;
}
/*!
    Creates a rotation matrix based on \a direction and \a up vectors.
*/
void Matrix4::direction(const Vector3 &direction, const Vector3 &up) {
    Vector3 z = direction;
    z.normalize();
    Vector3 x = up.cross(z);
    x.normalize();
    Vector3 y = z.cross(x);
    y.normalize();

    mat[0] = x.x;  mat[4] = x.y;  mat[8 ] = x.z;  mat[12] = 0.0f;
    mat[1] = y.x;  mat[5] = y.y;  mat[9 ] = y.z;  mat[13] = 0.0f;
    mat[2] = z.x;  mat[6] = z.y;  mat[10] = z.z;  mat[14] = 0.0f;
    mat[3] = 0.0f; mat[7] = 0.0f; mat[11] = 0.0f; mat[15] = 1.0f;
}
/*!
    Creates a perspective projection matrix.
    \a fov is the vertical field-of-view in degrees of the perspective matrix,
    \a aspect is the aspect ratio (width divided by height). \a znear and \a zfar set up the depth clipping planes.
*/
Matrix4 Matrix4::perspective(areal fov, areal aspect, areal znear, areal zfar) {
    float sine, cotangent;
    float radians = fov / 2 * DEG2RAD;

    sine = sin(radians);
    cotangent = cos(radians) / sine;

    Matrix4 result;

    result[0]  = cotangent / aspect;
    result[5]  = cotangent;
    result[10] = -(zfar + znear) / (zfar - znear);
    result[11] = -1;
    result[14] = -(2.0f * zfar * znear) / (zfar - znear);
    result[15] = 0;

    return result;
}
/*!
    Creates an orthogonal projection matrix.
    Creates a view showing the area between \a left, \a right, \a top and \a bottom, with \a znear and \a zfar set up the depth clipping planes.
*/
Matrix4 Matrix4::ortho(areal left, areal right, areal bottom, areal top, areal znear, areal zfar) {
    Matrix4 result;

    result[0]  =  2.0f / (right - left);
    result[5]  =  2.0f / (top - bottom);
    result[10] = -2.0f / (zfar - znear);
    result[12] = -((right + left) / (right - left));
    result[13] = -((top + bottom) / (top - bottom));
    result[14] = -((zfar + znear) / (zfar - znear));

    return result;
}
/*!
    Creates a transformation matrix that corresponds to a camera viewing the target from the source.
    Receiving \a eye point, a \a target point, and an \a up vector.
*/
Matrix4 Matrix4::lookAt(const Vector3 &eye, const Vector3 &target, const Vector3 &up) {
    Matrix4 m0, m1;

    m0.direction(eye - target, up);
    m1.translate(eye);

    return m0 * m1;
}
