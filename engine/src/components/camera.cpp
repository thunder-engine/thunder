#include "components/camera.h"

#include "components/actor.h"
#include "components/transform.h"

#include "resources/pipeline.h"

Camera *Camera::s_pCurrent  = nullptr;

Camera::Camera() {
    m_FOV       = 45.0; // 2*arctan(height/(2*distance))
    m_Near      = 0.1f;
    m_Far       = 1000.0;
    m_Ratio     = 1.0;
    m_Focal     = 1.0;
    m_OrthoWidth= 1.0;
    m_Ortho     = false;
    m_Color     = Vector4();
    m_pPipeline = nullptr;
}

Pipeline *Camera::pipeline() {
    if(m_pPipeline == nullptr) {
        m_pPipeline = Engine::objectCreate<Pipeline>("Pipeline");
    }
    return m_pPipeline;
}

void Camera::matrices(Matrix4 &v, Matrix4 &p) const {
    p   = projectionMatrix();

    Transform *t   = actor()->transform();
    Matrix4 m;
    m.translate(-t->position());
    v   = Matrix4(t->rotation().toMatrix()).inverse() * m;
}

Matrix4 Camera::projectionMatrix() const {
    if(m_Ortho) {
        float height    = m_OrthoWidth / m_Ratio;
        return Matrix4::ortho(-m_OrthoWidth / 2, m_OrthoWidth / 2, -height / 2, height / 2, m_Near, m_Far);
    }
    return Matrix4::perspective(m_FOV, m_Ratio, m_Near, m_Far);
}

bool Camera::project(const Vector3 &ws, const Matrix4 &modelview, const Matrix4 &projection, Vector3 &ss) {
    Vector4 in;
    Vector4 out;

    in  = Vector4(ws.x, ws.y, ws.z, 1.0f);
    out = modelview * in;
    in  = projection * out;

    if(in.w == 0.0f) {
        return false;
    }
    in.w    = 1.0f / in.w;
    in.x   *= in.w;
    in.y   *= in.w;
    in.z   *= in.w;

    ss.x    = (in.x * 0.5f + 0.5f);
    ss.y    = (in.y * 0.5f + 0.5f);
    ss.z    = (1.0f + in.z) * 0.5f;

    return true;
}

bool Camera::unproject(const Vector3 &ss, const Matrix4 &modelview, const Matrix4 &projection, Vector3 &ws) {
    Matrix4 final;
    Vector4 in;
    Vector4 out;

    final   = (projection * modelview).inverse();

    in.x    = (ss.x) * 2.0f - 1.0f;
    in.y    = (ss.y) * 2.0f - 1.0f;
    in.z    = 2.0f * ss.z - 1.0f;
    in.w    = 1.0f;
    out     = final * in;

    if(out.w == 0.0f) {
        return false;
    }
    out.w   = 1.0f / out.w;

    ws.x    = out.x * out.w;
    ws.y    = out.y * out.w;
    ws.z    = out.z * out.w;

    return true;
}

Ray Camera::castRay(float x, float y) {
    Actor *a        = actor();
    Vector3 p       = a->transform()->position();
    Vector3 dir     = a->transform()->rotation() * Vector3(0.0, 0.0,-1.0);
    dir.normalize();

    Vector3 view;
    if(m_Ortho) {
        p   = Vector3(p.x + (x - 0.5f) * m_OrthoWidth,
                      p.y - (y - 0.5f) * m_OrthoWidth / m_Ratio, p.z);
    } else {
        float tang      = tan(m_FOV * DEG2RAD);
        Vector3 right   = dir.cross(Vector3(0.0f, 1.0f, 0.0f)); /// \todo: Temp
        Vector3 up      = right.cross(dir);
        view    = Vector3( (x - 0.5f) * tang * m_Ratio) * right +
                  Vector3(-(y - 0.5f) * tang) * up + p + dir;

        dir = (view - p);
        dir.normalize();
    }

    return Ray (p, dir);
}

float Camera::fov() const {
    return m_FOV;
}

float Camera::nearPlane() const {
    return m_Near;
}

void Camera::setNear(float value) {
    m_Near  = value;
}

float Camera::farPlane() const {
    return m_Far;
}

void Camera::setFar(const float value) {
    m_Far   = value;
}

float Camera::ratio() const {
    return m_Ratio;
}
void Camera::setRatio(float value) {
    m_Ratio = value;
}

float Camera::focal() const {
    return m_Focal;
}

void Camera::setFocal(const float focal) {
    m_Focal = focal;
}

void Camera::setFov(const float value) {
    m_FOV   = value;
}

Vector4 Camera::color() const {
    return m_Color;
}

void Camera::setColor(const Vector4 &color) {
    m_Color = color;
}

float Camera::orthoWidth() const {
    return m_OrthoWidth;
}
void Camera::setOrthoWidth(const float value) {
    m_OrthoWidth    = value;
}

bool Camera::orthographic() const {
    return m_Ortho;
}

void Camera::setOrthographic(const bool value) {
    m_Ortho = value;
}

array<Vector3, 8> Camera::frustumCorners(float nearPlane, float farPlane) const {
    float nh;
    float fh;
    float nw;
    float fw;
    if(m_Ortho) {
        nw    = m_OrthoWidth * 0.5f;
        fw    = nw;
        nh    = nw / m_Ratio;
        fh    = nh;
    } else {
        float tang  = tanf(m_FOV * DEG2RAD * 0.5f);
        nh    = nearPlane * tang;
        fh    = farPlane * tang;
        nw    = nh * m_Ratio;
        fw    = fh * m_Ratio;
    }

    Transform *t    = actor()->transform();

    Vector3 pos     = t->worldPosition();
    Quaternion rot  = t->worldRotation();

    Vector3 dir     = rot * Vector3(0.0f, 0.0f,-1.0f);
    Vector3 right   = dir.cross(rot * Vector3(0.0f, 1.0f, 0.0f));
    Vector3 up      = right.cross(dir);
    Vector3 nc      = pos + dir * nearPlane;
    Vector3 fc      = pos + dir * farPlane;

    return {nc + up * nh - right * nw,
            nc + up * nh + right * nw,
            nc - up * nh + right * nw,
            nc - up * nh - right * nw,

            fc + up * fh - right * fw,
            fc + up * fh + right * fw,
            fc - up * fh + right * fw,
            fc - up * fh - right * fw};
}

Camera *Camera::current() {
    return s_pCurrent;
}

void Camera::setCurrent(Camera *current) {
    s_pCurrent  = current;
}
