#include "components/camera.h"

#include "components/actor.h"

Camera::Camera() {
    m_FOV       = 45.0; // 2*arctan(height/(2*distance))
    m_Near      = 0.1;
    m_Far       = 1000.0;
    m_Ratio     = 1.0;
    m_Focal     = 1.0;
    m_OrthoWidth= 1.0;
    m_Type      = PERSPECTIVE;
    m_Color     = Vector4();
}

void Camera::matrices(Matrix4 &v, Matrix4 &p) const {
    if(m_Type == PERSPECTIVE) {
        p   = Matrix4::perspective(m_FOV, m_Ratio, m_Near, m_Far);
    } else {
        float height    = m_OrthoWidth / m_Ratio;
        p   = Matrix4::ortho(-m_OrthoWidth / 2, m_OrthoWidth / 2, -height / 2, height / 2, m_Near, m_Far);
    }

    Actor &a   = actor();
    Matrix4 m;
    m.translate(-a.position());
    v   = Matrix4(a.rotation().toMatrix()).inverse() * m;
}

bool Camera::project(const Vector3 &ws, const Matrix4 &modelview, const Matrix4 &projection, Vector3 &ss) {
    Vector4 in;
    Vector4 out;

    in  = Vector4(ws.x, ws.y, ws.z, 1.0f);
    out = modelview * in;
    in  = projection * out;

    if(in.w == 0.0) {
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

    in.x    = (ss.x) * 2.0 - 1.0;
    in.y    = (ss.y) * 2.0 - 1.0;
    in.z    = 2.0f * ss.z - 1.0f;
    in.w    = 1.0f;
    out     = final * in;

    if(out.w == 0.0) {
        return false;
    }
    out.w   = 1.0f / out.w;

    ws.x    = out.x * out.w;
    ws.y    = out.y * out.w;
    ws.z    = out.z * out.w;

    return true;
}

Ray Camera::castRay(float x, float y) {
    float tang      = tan(m_FOV * DEG2RAD * 0.5);

    Actor &a        = actor();
    Vector3 p       = a.position();
    Vector3 dir     = a.rotation() * Vector3(0.0, 0.0,-1.0);
    dir.normalize();

    Vector3 right   = dir.cross(Vector3(0.0f, 1.0f, 0.0f)); /// \todo: Temp
    Vector3 up      = right.cross(dir);
    Vector3 view    = Vector3( (x - 0.5f) * tang * m_Ratio) * right +
                      Vector3(-(y - 0.5f) * tang) * up +
                      p + dir;

    dir = (view - p);
    dir.normalize();

    return Ray (p, dir);
}

double Camera::fov() const {
    return m_FOV;
}

double Camera::nearPlane() const {
    return m_Near;
}

void Camera::setNear(double value) {
    m_Near  = value;
}

double Camera::farPlane() const {
    return m_Far;
}

void Camera::setFar(const double value) {
    m_Far   = value;
}

double Camera::ratio() const {
    return m_Ratio;
}
void Camera::setRatio(double value) {
    m_Ratio = value;
}

double Camera::focal() const {
    return m_Focal;
}

void Camera::setFocal(const double focal) {
    m_Focal = focal;
}

Camera::Types Camera::type() const {
    return m_Type;
}

void Camera::setType(const Types type) {
    m_Type   = type;
}

void Camera::setFov(const double value) {
    m_FOV   = value;
}

Vector4 Camera::color() const {
    return m_Color;
}

void Camera::setColor(const Vector4 &color) {
    m_Color = color;
}

double Camera::orthoWidth() const {
    return m_OrthoWidth;
}
void Camera::setOrthoWidth(const double value) {
    m_OrthoWidth    = value;
}

array<Vector3, 8> Camera::frustumCorners(float nearPlane, float farPlane) const {
    float tang  = (float)tan(m_FOV * DEG2RAD * 0.5);
    float nh    = nearPlane * tang;
    float fh    = farPlane * tang;
    float nw    = nh * m_Ratio;
    float fw    = fh * m_Ratio;

    Vector3 dir     = Vector3(0.0, 0.0,-1.0);

    Vector3 right   = dir.cross(Vector3(0.0f, 1.0f, 0.0f));
    Vector3 up      = right.cross(dir);
    Vector3 nc      = dir * nearPlane;
    Vector3 fc      = dir * farPlane;

    return {nc + up * nh - right * nw,
            nc + up * nh + right * nw,
            nc - up * nh + right * nw,
            nc - up * nh - right * nw,
            fc + up * fh - right * fw,
            fc + up * fh + right * fw,
            fc - up * fh + right * fw,
            fc - up * fh - right * fw};
}
