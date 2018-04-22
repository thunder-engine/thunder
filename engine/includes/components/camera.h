#ifndef CAMERA_H
#define CAMERA_H

#include <array>

#include "component.h"

class NEXT_LIBRARY_EXPORT Camera : public Component {
    A_REGISTER(Camera, Component, Components);

    A_PROPERTIES(
        A_PROPERTY(float, Fov,  Camera::fov, Camera::setFov),
        A_PROPERTY(float, Near, Camera::nearPlane, Camera::setNear),
        A_PROPERTY(float, Far,  Camera::farPlane, Camera::setFar),
        A_PROPERTY(float, Size, Camera::orthoWidth, Camera::setOrthoWidth),
        A_PROPERTY(float, Focal_Distance, Camera::focal, Camera::setFocal),
        A_PROPERTY(Color, Background_Color, Camera::color, Camera::setColor)
    );

public:
    /*! \enum Types */
    enum Types {
        PERSPECTIVE     = 1,
        ORTHOGRAPHIC    = 2
    };

    Camera                      ();

    void                        matrices                (Matrix4 &v, Matrix4 &p) const;

    static bool                 project                 (const Vector3 &ws, const Matrix4 &modelview, const Matrix4 &projection, Vector3 &ss);
    static bool                 unproject               (const Vector3 &ss, const Matrix4 &modelview, const Matrix4 &projection, Vector3 &ws);

    Ray                         castRay                 (float x, float y);

    float                       ratio                   () const;
    void                        setRatio                (float value);

    Camera::Types               type                    () const;
    void                        setType                 (const Camera::Types type);

    float                       nearPlane               () const;
    void                        setNear                 (const float value);

    float                       farPlane                () const;
    void                        setFar                  (const float value);

    float                       focal                   () const;
    virtual void                setFocal                (const float focal);

    float                       fov                     () const;
    virtual void                setFov                  (const float value);

    Vector4                     color                   () const;
    void                        setColor                (const Vector4 &color);

    float                       orthoWidth              () const;
    void                        setOrthoWidth           (const float value);

    array<Vector3, 8>           frustumCorners          (float nearPlane, float farPlane) const;

protected:
    /// Type of camera.
    Types                       m_Type;
    /// Field Of View angle
    float                       m_FOV;
    /// Near plane of cut
    float                       m_Near;
    /// Far plane of cut
    float                       m_Far;
    /// Aspect ratio
    float                       m_Ratio;
    /// Focal distance
    float                       m_Focal;

    float                       m_OrthoWidth;

    Vector4                     m_Color;
};

#endif // CAMERA_H
