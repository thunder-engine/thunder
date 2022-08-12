#include "components/camera.h"

#include "components/actor.h"
#include "components/transform.h"

#include "resources/texture.h"

#include "pipelinecontext.h"

class CameraPrivate {
public:
    CameraPrivate() :
        m_ortho(false),
        m_fov(45.0), // 2*arctan(height/(2*distance))
        m_near(0.1f),
        m_far(1000.0f),
        m_ratio(1.0f),
        m_focal(1.0f),
        m_orthoSize(1.0f),
        m_color(Vector4()),
        m_pipeline(nullptr) {

    }

    static inline bool intersect(Plane pl[6], Vector3 points[8]) {
        for(int i = 0; i < 6; i++) {
            if(pl[i].sqrDistance(points[0]) > 0) {
                continue;
            }
            if(pl[i].sqrDistance(points[1]) > 0) {
                continue;
            }
            if(pl[i].sqrDistance(points[2]) > 0) {
                continue;
            }
            if(pl[i].sqrDistance(points[3]) > 0) {
                continue;
            }
            if(pl[i].sqrDistance(points[4]) > 0) {
                continue;
            }
            if(pl[i].sqrDistance(points[5]) > 0) {
                continue;
            }
            if(pl[i].sqrDistance(points[6]) > 0) {
                continue;
            }
            if(pl[i].sqrDistance(points[7]) > 0) {
                continue;
            }
            return false;
        }
        return true;
    }

    bool m_ortho;

    float m_fov;

    float m_near;

    float m_far;

    float m_ratio;

    float m_focal;

    float m_orthoSize;

    Vector4 m_color;

    static Camera *s_current;

    PipelineContext *m_pipeline;

};

Camera *CameraPrivate::s_current  = nullptr;

/*!
    \class Camera
    \brief The Camera represents the player's point of view; how the player sees the world.
    \inmodule Engine

*/

Camera::Camera() :
    p_ptr(new CameraPrivate) {

}

Camera::~Camera() {
    delete p_ptr;
    p_ptr = nullptr;
}
/*!
    Returns render pipline which attached to the camera.
*/
PipelineContext *Camera::pipeline() {
    if(p_ptr->m_pipeline == nullptr) {
        p_ptr->m_pipeline = Engine::objectCreate<PipelineContext>("Pipeline");
    }
    return p_ptr->m_pipeline;
}
/*!
    Attaches render \a pipeline to the camera.
*/
void Camera::setPipeline(PipelineContext *pipeline) {
    p_ptr->m_pipeline = pipeline;
}
/*!
    Returns view matrix for the camera.
*/
Matrix4 Camera::viewMatrix() const {
    Transform *t = actor()->transform();
    Matrix4 m;
    m.translate(-t->position());
    return Matrix4(t->quaternion().toMatrix()).inverse() * m;
}
/*!
    Returns projection matrix for the camera.
*/
Matrix4 Camera::projectionMatrix() const {
    if(p_ptr->m_ortho) {
        float width = p_ptr->m_orthoSize * p_ptr->m_ratio;
        return Matrix4::ortho(-width / 2, width / 2, -p_ptr->m_orthoSize / 2, p_ptr->m_orthoSize / 2, p_ptr->m_near, p_ptr->m_far);
    }
    return Matrix4::perspective(p_ptr->m_fov, p_ptr->m_ratio, p_ptr->m_near, p_ptr->m_far);
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
    out.w = 1.0f / out.w;

    return Vector3(out.x * out.w, out.y * out.w, out.z * out.w);
}
/*!
    Returns ray with origin point in camera position and direction to projection plane with \a x and \a y coordinates.
*/
Ray Camera::castRay(float x, float y) {
    Transform *t = actor()->transform();
    Vector3 p   = t->position();
    Vector3 dir = t->quaternion() * Vector3(0.0, 0.0,-1.0);
    dir.normalize();

    Vector3 view;
    if(p_ptr->m_ortho) {
        p += t->quaternion() * Vector3((x - 0.5f) * p_ptr->m_orthoSize * p_ptr->m_ratio,
                                       (y - 0.5f) * p_ptr->m_orthoSize, 0.0f);
    } else {
        float tang    = tan(p_ptr->m_fov * DEG2RAD);
        Vector3 right = dir.cross(Vector3(0.0f, 1.0f, 0.0f));
        Vector3 up    = right.cross(dir);
        view = Vector3((x - 0.5f) * tang * p_ptr->m_ratio) * right +
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
    return p_ptr->m_fov;
}
/*!
    Sets field of view \a angle for the camera in degrees.
    \note Applicable only for the perspective mode.
*/
void Camera::setFov(const float angle) {
    p_ptr->m_fov   = angle;
}
/*!
    Returns a distance to near cut plane.
*/
float Camera::nearPlane() const {
    return p_ptr->m_near;
}
/*!
    Sets a \a distance to near cut plane.
*/
void Camera::setNear(const float distance) {
    p_ptr->m_near = distance;
}
/*!
    Returns a distance to far cut plane.
*/
float Camera::farPlane() const {
    return p_ptr->m_far;
}
/*!
    Sets a \a distance to far cut plane.
*/
void Camera::setFar(const float distance) {
    p_ptr->m_far = distance;
}
/*!
    Returns the aspect ratio (width divided by height).
*/
float Camera::ratio() const {
    return p_ptr->m_ratio;
}
/*!
    Sets the aspect \a ratio (width divided by height).
*/
void Camera::setRatio(float ratio) {
    p_ptr->m_ratio = ratio;
}
/*!
    Returns a focal distance for the camera.
*/
float Camera::focal() const {
    return p_ptr->m_focal;
}
/*!
    Sets a \a focal distance for the camera.
*/
void Camera::setFocal(const float focal) {
    p_ptr->m_focal = focal;
}
/*!
    Returns the color with which the screen will be cleared.
*/
Vector4 &Camera::color() const {
    return p_ptr->m_color;
}
/*!
    Sets the \a color with which the screen will be cleared.
*/
void Camera::setColor(const Vector4 color) {
    p_ptr->m_color = color;
}
/*!
    Returns camera size for orthographic mode.
*/
float Camera::orthoSize() const {
    return p_ptr->m_orthoSize;
}
/*!
    Sets camera \a size for orthographic mode.
*/
void Camera::setOrthoSize(const float size) {
    p_ptr->m_orthoSize = size;
}
/*!
    Returns true for the orthographic mode; for the perspective mode, returns false.
*/
bool Camera::orthographic() const {
    return p_ptr->m_ortho;
}
/*!
    Sets orthographic \a mode.
*/
void Camera::setOrthographic(const bool mode) {
    p_ptr->m_ortho = mode;
}
/*!
    Returns current active camera.
*/
Camera *Camera::current() {
    return CameraPrivate::s_current;
}
/*!
    Sets \a current active camera.
*/
void Camera::setCurrent(Camera *current) {
    CameraPrivate::s_current = current;
}
/*!
    Returns frustum corners for the \a camera.
*/
array<Vector3, 8> Camera::frustumCorners(const Camera &camera) {
    Transform *t = camera.actor()->transform();

    return Camera::frustumCorners(camera.p_ptr->m_ortho, (camera.p_ptr->m_ortho) ?
                                                          camera.p_ptr->m_orthoSize :
                                                          camera.p_ptr->m_fov,
                                                          camera.p_ptr->m_ratio,
                                  t->worldPosition(), t->worldRotation(),
                                  camera.p_ptr->m_near, camera.p_ptr->m_far);
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
        array<Vector3, 8> a = frustumCorners(p_ptr->m_ortho, (p_ptr->m_ortho) ? p_ptr->m_orthoSize : p_ptr->m_fov,
                                             p_ptr->m_ratio, t->worldPosition(), t->worldRotation(), nearPlane(), farPlane());

        Handles::s_Color = Vector4(0.5f, 0.5f, 0.5f, 1.0f);
        Vector3Vector points(a.begin(), a.end());
        IndexVector indices   = {0, 1, 1, 2, 2, 3, 3, 0,
                                 4, 5, 5, 6, 6, 7, 7, 4,
                                 0, 4, 1, 5, 2, 6, 3, 7};

        Handles::drawLines(Matrix4(), points, indices);
    }
    Handles::s_Color = Handles::s_Normal;
    return Handles::drawBillboard(t->position(), Vector2(0.5f), Engine::loadResource<Texture>(".embedded/camera.png"));
}
#endif
