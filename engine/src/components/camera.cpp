#include "components/camera.h"

#include "components/actor.h"
#include "components/transform.h"

#include "pipelinecontext.h"
#include "gizmos.h"

#include <float.h>

Camera *s_currentCamera  = nullptr;

/*!
    \class Camera
    \brief The Camera represents the player's point of view; how the player sees the world.
    \inmodule Components
*/

Camera::Camera() :
        m_fov(45.0), // 2*arctan(height/(2*distance))
        m_near(0.1f),
        m_far(1000.0f),
        m_ratio(1.0f),
        m_focal(1.0f),
        m_orthoSize(1.0f),
        m_ortho(false),
        m_screen(false) {

    recalcProjection();
}

/*!
    Returns view matrix for the camera.
*/
Matrix4 Camera::viewMatrix() const {
    Transform *t = transform();
    Matrix4 m;
    m.translate(-t->worldPosition());
    return Matrix4(t->worldQuaternion().toMatrix()).inverse() * m;
}
/*!
    Returns projection matrix for the camera.
*/
Matrix4 Camera::projectionMatrix() const {
    return m_projection;
}
/*!
    Transforms position from \a worldSpace into screen space.
    Returns result of transformation.
*/
Vector3 Camera::project(const Vector3 &worldSpace) {
    Vector4 in(worldSpace.x, worldSpace.y, worldSpace.z, 1.0f);
    Vector4 out(viewMatrix() * in);
    in = m_projection * out;

    if(in.w == 0.0f) {
        return Vector3(); // false;
    }
    in.w  = 1.0f / in.w;
    in.x *= in.w;
    in.y *= in.w;
    in.z *= in.w;

    return Vector3((in.x * 0.5f + 0.5f), (in.y * 0.5f + 0.5f), in.z);
}
/*!
    Transforms position from \a screenSpace into world space.
    Returns result of transformation.
*/
Vector3 Camera::unproject(const Vector3 &screenSpace) {
    Matrix4 final((m_projection * viewMatrix()).inverse());

    Vector4 in(2.0f * screenSpace.x - 1.0f,
               2.0f * screenSpace.y - 1.0f,
               2.0f * screenSpace.z - 1.0f,
               1.0f);

    Vector4 out(final * in);

    if(out.w == 0.0f) {
        return Vector3(); // false
    }

    return Vector3(out.x / out.w, out.y / out.w, out.z / out.w);
}
/*!
    Returns ray with origin point in camera position and direction to projection plane with \a x and \a y coordinates.
*/
Ray Camera::castRay(float x, float y) {
    Transform *t = transform();

    Vector3 p(t->worldPosition());
    Vector3 dir;

    if(m_ortho) {
        Quaternion q(t->worldQuaternion());

        dir = q * Vector3(0.0f, 0.0f,-1.0f);
        p += q * Vector3((x - 0.5f) * m_orthoSize * m_ratio,
                         (y - 0.5f) * m_orthoSize, 0.0f);
    } else {
        dir = unproject(Vector3(x, y, 1.0f)) - p;
    }

    dir.normalize();

    return Ray(p, dir);
}
/*!
    Returns field of view angle for the camera in degrees.
*/
float Camera::fov() const {
    return m_fov;
}
/*!
    Sets field of view \a angle for the camera in degrees.
    \note Applicable only for the perspective mode.
*/
void Camera::setFov(const float angle) {
    m_fov = angle;
    recalcProjection();
}
/*!
    Returns a distance to near cut plane.
*/
float Camera::nearPlane() const {
    return m_near;
}
/*!
    Sets a \a distance to near cut plane.
*/
void Camera::setNear(const float distance) {
    m_near = distance;
    recalcProjection();
}
/*!
    Returns a distance to far cut plane.
*/
float Camera::farPlane() const {
    return m_far;
}
/*!
    Sets a \a distance to far cut plane.
*/
void Camera::setFar(const float distance) {
    m_far = distance;
    recalcProjection();
}
/*!
    Returns the aspect ratio (width divided by height).
*/
float Camera::ratio() const {
    return m_ratio;
}
/*!
    Sets the aspect \a ratio (width divided by height).
*/
void Camera::setRatio(float ratio) {
    m_ratio = ratio;
    recalcProjection();
}
/*!
    Returns a focal distance for the camera.
*/
float Camera::focal() const {
    return m_focal;
}
/*!
    Sets a \a focal distance for the camera.
*/
void Camera::setFocal(const float focal) {
    m_focal = focal;
    recalcProjection();
}
/*!
    Returns the color with which the screen will be cleared.
*/
Vector4 Camera::color() const {
    return m_color;
}
/*!
    Sets the \a color with which the screen will be cleared.
*/
void Camera::setColor(const Vector4 color) {
    m_color = color;
}
/*!
    Returns camera size for orthographic mode.
*/
float Camera::orthoSize() const {
    return m_orthoSize;
}
/*!
    Sets camera \a size for orthographic mode.
*/
void Camera::setOrthoSize(const float size) {
    m_orthoSize = CLAMP(size, FLT_EPSILON, 100000.0f);
    recalcProjection();
}
/*!
    Returns true for the orthographic mode; for the perspective mode, returns false.
*/
bool Camera::orthographic() const {
    return m_ortho;
}
/*!
    Sets orthographic \a mode.
*/
void Camera::setOrthographic(const bool mode) {
    m_ortho = mode;
    recalcProjection();
}
/*!
    Returns true is this camera in the screen space mode.
    Typically used for Editor.
*/
bool Camera::isScreenSpace() const {
    return m_screen;
}
/*!
    Sets the screen space \a mode for the camera.
    Typically used for Editor.
*/
void Camera::setScreenSpace(bool mode) {
    m_screen = mode;
    recalcProjection();
}
/*!
    Returns current active camera.
*/
Camera *Camera::current() {
    return s_currentCamera;
}
/*!
    Sets \a current active camera.
*/
void Camera::setCurrent(Camera *current) {
    s_currentCamera = current;
}

Frustum Camera::frustum() const {
    Transform *t = transform();

    return Camera::frustum(m_ortho, m_ortho ? m_orthoSize : m_fov, m_ratio,
                           t->worldPosition(), t->worldRotation(),
                           m_near, m_far);
}

/*!
    Returns frustum corners for the \a camera.
*/
std::array<Vector3, 8> Camera::frustumCorners(const Camera &camera) {
    Transform *t = camera.transform();

    return Camera::frustumCorners(camera.m_ortho, camera.m_ortho ? camera.m_orthoSize : camera.m_fov, camera.m_ratio,
                                  t->worldPosition(), t->worldRotation(),
                                  camera.m_near, camera.m_far);
}
/*!
    Returns frustum corners with provided parameters.
    This function accepts a list of parameters:
    \a ortho is a flag that points orthographic or perspective camera.
    \a sigma is an angle of frustum or ortho size in the case of an orthographic camera.
    \a ratio is an aspect ratio.
    \a position of the frustum in world space.
    \a rotation of frustum in world space.
    \a nearPlane clipping plane.
    \a farPlane clipping plane.
*/
std::array<Vector3, 8> Camera::frustumCorners(bool ortho, float sigma, float ratio, const Vector3 &position,
                                         const Quaternion &rotation, float nearPlane, float farPlane) {
    float nh;
    float fh;
    float nw;
    float fw;
    if(ortho) {
        nh = sigma * 0.5f;
        nw = nh * ratio;
        fw = nw;
        fh = nh;
    } else {
        float tang = tanf(sigma * DEG2RAD * 0.5f);
        nh = nearPlane * tang;
        fh = farPlane * tang;
        nw = nh * ratio;
        fw = fh * ratio;
    }

    Vector3 dir = rotation * Vector3(0.0f, 0.0f,-1.0f);
    Vector3 right = dir.cross(rotation * Vector3(0.0f, 1.0f, 0.0f));
    Vector3 up = right.cross(dir);

    Vector3 nc(position + dir * nearPlane);
    Vector3 fc(position + dir * farPlane);

    return {nc + up * nh - right * nw,
            nc + up * nh + right * nw,
            nc - up * nh + right * nw,
            nc - up * nh - right * nw,

            fc + up * fh - right * fw,
            fc + up * fh + right * fw,
            fc - up * fh + right * fw,
            fc - up * fh - right * fw};
}

Frustum Camera::frustum(bool ortho, float sigma, float ratio, const Vector3 &position,
                        const Quaternion &rotation, float nearPlane, float farPlane) {

    std::array<Vector3, 8> points = frustumCorners(ortho, sigma, ratio, position, rotation, nearPlane, farPlane);

    Frustum result;

    result.m_top = Plane(points[1], points[0], points[4]);
    result.m_bottom = Plane(points[7], points[3], points[2]);
    result.m_left = Plane(points[3], points[7], points[0]);
    result.m_right = Plane(points[2], points[1], points[6]);
    result.m_near = Plane(points[0], points[1], points[3]);
    result.m_far = Plane(points[5], points[4], points[6]);

    return result;
}

void Camera::drawGizmos() {
    Transform *t = transform();

    Gizmos::drawIcon(t->worldPosition(), Vector2(0.5f), ".embedded/camera.png", Vector4(1.0f));
}

void Camera::drawGizmosSelected() {
    Transform *t = transform();
    std::array<Vector3, 8> a = frustumCorners(m_ortho, (m_ortho) ? m_orthoSize : m_fov,
                                         m_ratio, t->worldPosition(), t->worldQuaternion(), nearPlane(), farPlane());

    Vector3Vector points(a.begin(), a.end());
    IndexVector indices = {0, 1, 1, 2, 2, 3, 3, 0,
                           4, 5, 5, 6, 6, 7, 7, 4,
                           0, 4, 1, 5, 2, 6, 3, 7};

    Gizmos::drawLines(points, indices, Vector4(0.5f, 0.5f, 0.5f, 1.0f));
}

void Camera::recalcProjection() {
    if(m_ortho) {
        float width = m_orthoSize * m_ratio;
        if(m_screen) {
            m_projection = Matrix4::ortho(0, width, 0, m_orthoSize, m_near, m_far);
        } else {
            m_projection = Matrix4::ortho(-width / 2, width / 2, -m_orthoSize / 2, m_orthoSize / 2, m_near, m_far);
        }
    } else {
        m_projection = Matrix4::perspective(m_fov, m_ratio, m_near, m_far);
    }
}
