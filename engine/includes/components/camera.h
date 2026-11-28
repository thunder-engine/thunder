/*
    This file is part of Thunder Engine.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#ifndef CAMERA_H
#define CAMERA_H

#include <array>

#include <component.h>

class ENGINE_EXPORT Camera : public Component {
    A_OBJECT(Camera, Component, Components)

    A_PROPERTIES(
        A_PROPERTY(float, fov,  Camera::fov, Camera::setFov),
        A_PROPERTY(float, near, Camera::nearPlane, Camera::setNear),
        A_PROPERTY(float, far,  Camera::farPlane, Camera::setFar),
        A_PROPERTY(float, size, Camera::orthoSize, Camera::setOrthoSize),
        A_PROPERTY(float, focalDistance, Camera::focalDistance, Camera::setFocalDistance),
        A_PROPERTYEX(Vector4, backgroundColor, Camera::color, Camera::setColor, "editor=Color"),
        A_PROPERTY(bool, orthographic, Camera::orthographic, Camera::setOrthographic)
    )

    A_METHODS(
        A_METHOD(Matrix4, Camera::viewMatrix),
        A_METHOD(Matrix4, Camera::projectionMatrix),
        A_METHOD(Vector3, Camera::project),
        A_METHOD(Vector3, Camera::unproject),
        A_STATIC(Camera *, Camera::current),
        A_STATIC(void, Camera::setCurrent),
        A_METHOD(Ray, Camera::castRay)
    )

public:
    Camera();
    ~Camera();

    Matrix4 viewMatrix() const;
    Matrix4 projectionMatrix() const;

    Ray castRay(float x, float y);

    float ratio() const;
    void setRatio(float ratio);

    float nearPlane() const;
    void setNear(const float distance);

    float farPlane() const;
    void setFar(const float distance);

    float focalDistance() const;
    void setFocalDistance(const float focal);

    float fov() const;
    void setFov(const float angle);

    Vector4 color() const;
    void setColor(const Vector4 color);

    float orthoSize() const;
    void setOrthoSize(const float size);

    bool orthographic() const;
    void setOrthographic(const bool mode);

    bool isScreenSpace() const;
    void setScreenSpace(bool mode);

    static Camera *current();
    static void setCurrent(Camera *current);

    static Camera *findActiveCamera(World *world);

    Vector3 project(const Vector3 &worldSpace) const;
    Vector3 unproject(const Vector3 &screenSpace) const;

    Frustum frustum() const;

    static std::array<Vector3, 8> frustumCorners(const Camera &camera);
    static std::array<Vector3, 8> frustumCorners(bool ortho, float sigma, float ratio, const Vector3 &position, const Quaternion &rotation, float nearPlane, float farPlane);

    static Frustum frustum(bool ortho, float sigma, float ratio, const Vector3 &position, const Quaternion &rotation, float nearPlane, float farPlane);

private:
    void drawGizmos() override;
    void drawGizmosSelected() override;

private:
    mutable Matrix4 m_projection;

    Vector4 m_color;

    float m_fov;

    float m_near;

    float m_far;

    float m_ratio;

    float m_focal;

    float m_orthoSize;

    bool m_ortho;

    bool m_screen;

    bool m_main;

    mutable bool m_dirty;

};

#endif // CAMERA_H
