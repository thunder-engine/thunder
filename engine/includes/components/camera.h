#ifndef CAMERA_H
#define CAMERA_H

#include <array>

#include "nativebehaviour.h"

class Pipeline;
class CameraPrivate;

class NEXT_LIBRARY_EXPORT Camera : public Component {
    A_REGISTER(Camera, Component, Components)

    A_PROPERTIES(
        A_PROPERTY(float, Fov,  Camera::fov, Camera::setFov),
        A_PROPERTY(float, Near, Camera::nearPlane, Camera::setNear),
        A_PROPERTY(float, Far,  Camera::farPlane, Camera::setFar),
        A_PROPERTY(float, Size, Camera::orthoHeight, Camera::setOrthoHeight),
        A_PROPERTY(float, Focal_Distance, Camera::focal, Camera::setFocal),
        A_PROPERTY(Color, Background_Color, Camera::color, Camera::setColor),
        A_PROPERTY(bool, Orthographic, Camera::orthographic, Camera::setOrthographic)
    )
    A_NOMETHODS()

public:
    Camera ();
    ~Camera ();

    Pipeline *pipeline ();

    void setPipeline (Pipeline *pipeline);

    void matrices (Matrix4 &v, Matrix4 &p) const;

    Matrix4 projectionMatrix () const;

    static bool project (const Vector3 &ws, const Matrix4 &modelview, const Matrix4 &projection, Vector3 &ss);
    static bool unproject (const Vector3 &ss, const Matrix4 &modelview, const Matrix4 &projection, Vector3 &ws);

    Ray castRay (float x, float y);

    float ratio () const;
    void setRatio (float value);

    float nearPlane () const;
    void setNear (const float value);

    float farPlane () const;
    void  setFar (const float value);

    float focal () const;
    void setFocal (const float focal);

    float fov () const;
    void setFov (const float value);

    Vector4 color () const;
    void setColor (const Vector4 &color);

    float orthoHeight () const;
    void setOrthoHeight (const float value);

    bool orthographic () const;
    void setOrthographic (const bool value);

    static Camera *current ();
    static void setCurrent (Camera *current);

    static array<Vector3, 8> frustumCorners (bool ortho, float sigma, float ratio, const Vector3 &position, const Quaternion &rotation, float nearPlane, float farPlane);
    static Object::ObjectList Camera::frustumCulling(ObjectList &in, const array<Vector3, 8> &frustum);

private:
#ifdef NEXT_SHARED
    bool drawHandles() override;
#endif

private:
    CameraPrivate *p_ptr;

};

#endif // CAMERA_H
