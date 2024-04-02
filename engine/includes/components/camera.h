#ifndef CAMERA_H
#define CAMERA_H

#include <array>

#include "renderable.h"

class ENGINE_EXPORT Camera : public Component {
    A_REGISTER(Camera, Component, Components)

    A_PROPERTIES(
        A_PROPERTY(float, fov,  Camera::fov, Camera::setFov),
        A_PROPERTY(float, near, Camera::nearPlane, Camera::setNear),
        A_PROPERTY(float, far,  Camera::farPlane, Camera::setFar),
        A_PROPERTY(float, size, Camera::orthoSize, Camera::setOrthoSize),
        A_PROPERTY(float, focalDistance, Camera::focal, Camera::setFocal),
        A_PROPERTYEX(Vector4, backgroundColor, Camera::color, Camera::setColor, "editor=Color"),
        A_PROPERTY(bool, orthographic, Camera::orthographic, Camera::setOrthographic),
        A_PROPERTY(bool, ratio, Camera::ratio, Camera::setRatio)
    )

    A_METHODS(
        A_METHOD(Matrix4, Camera::viewMatrix),
        A_METHOD(Matrix4, Camera::projectionMatrix),
        A_STATIC(Vector3, Camera::project),
        A_STATIC(Vector3, Camera::unproject),
        A_STATIC(Camera *, Camera::current),
        A_STATIC(void, Camera::setCurrent),
        A_METHOD(Ray, Camera::castRay)
    )

public:
    Camera();

    Matrix4 viewMatrix() const;
    Matrix4 projectionMatrix() const;

    Ray castRay(float x, float y);

    float ratio() const;
    void setRatio(float ratio);

    float nearPlane() const;
    void setNear(const float distance);

    float farPlane() const;
    void setFar(const float distance);

    float focal() const;
    void setFocal(const float focal);

    float fov() const;
    void setFov(const float angle);

    Vector4 color() const;
    void setColor(const Vector4 color);

    float orthoSize() const;
    void setOrthoSize(const float size);

    bool orthographic() const;
    void setOrthographic(const bool mode);

    static Camera *current();
    static void setCurrent(Camera *current);

    static Vector3 project(const Vector3 &worldSpace, const Matrix4 &modelView, const Matrix4 &projection);
    static Vector3 unproject(const Vector3 &screenSpace, const Matrix4 &modelView, const Matrix4 &projection);

    static array<Vector3, 8> frustumCorners(const Camera &camera);
    static array<Vector3, 8> frustumCorners(bool ortho, float sigma, float ratio, const Vector3 &position, const Quaternion &rotation, float nearPlane, float farPlane);

private:
    void drawGizmos() override;
    void drawGizmosSelected() override;

private:
    bool m_ortho;

    float m_fov;

    float m_near;

    float m_far;

    float m_ratio;

    float m_focal;

    float m_orthoSize;

    Vector4 m_color;

};

#endif // CAMERA_H
