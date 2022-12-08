#include "components/camera.h"

#include "components/actor.h"
#include "components/transform.h"

#include "resources/texture.h"

#include "pipelinecontext.h"

Camera *s_currentCamera  = nullptr;

/*!
    \class Camera
    \brief The Camera represents the player's point of view; how the player sees the world.
    \inmodule Engine

*/

Camera::Camera() :
    m_ortho(false),
    m_fov(45.0), // 2*arctan(height/(2*distance))
    m_near(0.1f),
    m_far(1000.0f),
    m_ratio(1.0f),
    m_focal(1.0f),
    m_orthoSize(1.0f),
    m_color(Vector4()) {

}

Camera::~Camera() {

}
/*!
    Returns view matrix for the camera.
*/
Matrix4 Camera::viewMatrix() const {
    Transform *t = actor()->transform();
    Matrix4 m;
    m.translate(-t->worldPosition());
    return Matrix4(t->worldQuaternion().toMatrix()).inverse() * m;
}
/*!
    Returns projection matrix for the camera.
*/
Matrix4 Camera::projectionMatrix() const {
    if(m_ortho) {
        float width = m_orthoSize * m_ratio;
        return Matrix4::ortho(-width / 2, width / 2, -m_orthoSize / 2, m_orthoSize / 2, m_near, m_far);
    }
    return Matrix4::perspective(m_fov, m_ratio, m_near, m_far);
}
/*!
    Transforms position from \a worldSpace into screen space using \a modelView and \a projection matrices.
    Returns result of transformation.
*/
Vector3 Camera::project(const Vector3 &worldSpace, const Matrix4 &modelView, const Matrix4 &projection) {
    Vector4 in;
    Vector4 out;

    in  = Vector4(worldSpace.x, worldSpace.y, worldSpace.z, 1.0f);
    out = modelView * in;
    in  = projection * out;

    if(in.w == 0.0f) {
        return Vector3(); // false;
    }
    in.w  = 1.0f / in.w;
    in.x *= in.w;
    in.y *= in.w;
    in.z *= in.w;

    return Vector3((in.x * 0.5f + 0.5f), (in.y * 0.5f + 0.5f), (1.0f + in.z) * 0.5f);
}
/*!
    Transforms position from \a screenSpace into world space using \a modelView and \a projection matrices.
    Returns result of transformation.
*/
Vector3 Camera::unproject(const Vector3 &screenSpace, const Matrix4 &modelView, const Matrix4 &projection) {
    Matrix4 final;
    Vector4 in;
    Vector4 out;

    final = (projection * modelView).inverse();

    in.x = (screenSpace.x) * 2.0f - 1.0f;
    in.y = (screenSpace.y) * 2.0f - 1.0f;
    in.z = 2.0f * screenSpace.z - 1.0f;
    in.w = 1.0f;
    out  = final * in;

    if(out.w == 0.0f) {
        return Vector3(); // false
    }

    return Vector3(out.x / out.w, out.y / out.w, out.z / out.w);
}
/*!
    Returns ray with origin point in camera position and direction to projection plane with \a x and \a y coordinates.
*/
Ray Camera::castRay(float x, float y) {
    Transform *t = actor()->transform();
    Vector3 p   = t->worldPosition();
    Vector3 dir = t->worldQuaternion() * Vector3(0.0, 0.0,-1.0);
    dir.normalize();

    Vector3 view;
    if(m_ortho) {
        p += t->worldQuaternion() * Vector3((x - 0.5f) * m_orthoSize * m_ratio,
                                            (y - 0.5f) * m_orthoSize, 0.0f);
    } else {
        float tang    = tan(m_fov * DEG2RAD);
        Vector3 right = dir.cross(Vector3(0.0f, 1.0f, 0.0f));
        Vector3 up    = right.cross(dir);
        view = Vector3((x - 0.5f) * tang * m_ratio) * right +
               Vector3((y - 0.5f) * tang) * up + p + dir;

        dir = (view - p);
        dir.normalize();
    }
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
    m_fov   = angle;
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
    m_orthoSize = size;
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
/*!
    Returns frustum corners for the \a camera.
*/
array<Vector3, 8> Camera::frustumCorners(const Camera &camera) {
    Transform *t = camera.actor()->transform();

    return Camera::frustumCorners(camera.m_ortho, (camera.m_ortho) ?
                                                          camera.m_orthoSize :
                                                          camera.m_fov,
                                                          camera.m_ratio,
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
array<Vector3, 8> Camera::frustumCorners(bool ortho, float sigma, float ratio, const Vector3 &position,
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
        float tang  = tanf(sigma * DEG2RAD * 0.5f);
        nh = nearPlane * tang;
        fh = farPlane * tang;
        nw = nh * ratio;
        fw = fh * ratio;
    }

    Vector3 dir   = rotation * Vector3(0.0f, 0.0f,-1.0f);
    Vector3 right = dir.cross(rotation * Vector3(0.0f, 1.0f, 0.0f));
    Vector3 up    = right.cross(dir);
    Vector3 nc    = position + dir * nearPlane;
    Vector3 fc    = position + dir * farPlane;

    return {nc + up * nh - right * nw,
            nc + up * nh + right * nw,
            nc - up * nh + right * nw,
            nc - up * nh - right * nw,

            fc + up * fh - right * fw,
            fc + up * fh + right * fw,
            fc - up * fh + right * fw,
            fc - up * fh - right * fw};
}
/*!
    Filters out an incoming \a list which are not in the \a frustum.
    Returns filtered list.
*/
RenderList Camera::frustumCulling(RenderList &list, const array<Vector3, 8> &frustum) {
    Plane pl[6];
    pl[0] = Plane(frustum[1], frustum[0], frustum[4]); // top
    pl[1] = Plane(frustum[7], frustum[3], frustum[2]); // bottom
    pl[2] = Plane(frustum[3], frustum[7], frustum[0]); // left
    pl[3] = Plane(frustum[2], frustum[1], frustum[6]); // right
    pl[4] = Plane(frustum[0], frustum[1], frustum[3]); // near
    pl[5] = Plane(frustum[5], frustum[4], frustum[6]); // far

    RenderList result;
    for(auto it : list) {
        AABBox box = it->bound();
        if(box.extent.x < 0.0f || box.intersect(pl, 6)) {
            result.push_back(it);
        }
    }
    return result;
}

#ifdef SHARED_DEFINE
#include "viewport/handles.h"

bool Camera::drawHandles(ObjectList &selected) {
    Transform *t = actor()->transform();
    if(isSelected(selected)) {
        array<Vector3, 8> a = frustumCorners(m_ortho, (m_ortho) ? m_orthoSize : m_fov,
                                             m_ratio, t->worldPosition(), t->worldRotation(), nearPlane(), farPlane());

        Handles::s_Color = Vector4(0.5f, 0.5f, 0.5f, 1.0f);
        Vector3Vector points(a.begin(), a.end());
        IndexVector indices   = {0, 1, 1, 2, 2, 3, 3, 0,
                                 4, 5, 5, 6, 6, 7, 7, 4,
                                 0, 4, 1, 5, 2, 6, 3, 7};

        Handles::drawLines(Matrix4(), points, indices);
    }
    Handles::s_Color = Handles::s_Normal;
    return Handles::drawBillboard(t->worldPosition(), Vector2(0.5f), Engine::loadResource<Texture>(".embedded/camera.png"));
}
#endif
