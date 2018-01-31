#ifndef CAMERA_H
#define CAMERA_H

#include <array>

#include "component.h"

class NEXT_LIBRARY_EXPORT Camera : public Component {
    A_REGISTER(Camera, Component, Components)

    A_PROPERTIES(
        A_PROPERTY(double, Fov, Camera::fov, Camera::setFov),
        A_PROPERTY(double, Near, Camera::nearPlane, Camera::setNear),
        A_PROPERTY(double, Far, Camera::farPlane, Camera::setFar),
        A_PROPERTY(Color, Background_color, Camera::color, Camera::setColor)
    )

public:
    /*! \enum Types */
    enum Types {
        PERSPECTIVE     = 1,
        ORTHOGRAPHIC    = 2
    };

    Camera                      ();

    void                        matrices                (Matrix4 &v, Matrix4 &p) const;

    static bool                 project                 (const Vector3 &ws, const Matrix4 &modelview, const Matrix4 &projection, int viewport[4], Vector3 &ss);
    static bool                 unproject               (const Vector3 &ss, const Matrix4 &modelview, const Matrix4 &projection, int viewport[4], Vector3 &ws);

    Ray                         castRay                 (float x, float y);

    virtual void                resize                  (uint32_t w, uint32_t h);

    uint32_t                    width                   () const;
    uint32_t                    height                  () const;
    double                      ratio                   () const;

    Camera::Types               type                    () const;
    void                        setType                 (const Camera::Types type);

    double                      nearPlane               () const;
    void                        setNear                 (const double value);

    double                      farPlane                () const;
    void                        setFar                  (const double value);

    double                      focal                   () const;
    virtual void                setFocal                (const double focal);

    double                      fov                     () const;
    virtual void                setFov                  (const double value);

    Vector4                     color                   () const;
    void                        setColor                (const Vector4 &color);

    double                      orthoWidth              () const;
    void                        setOrthoWidth           (const double value);

    array<Vector3, 4>           frustumCorners          (float depth) const;

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

    uint32_t                    m_Width;
    uint32_t                    m_Height;

};

#endif // CAMERA_H
