#ifndef LIGHTSOURCE_H
#define LIGHTSOURCE_H

#include "component.h"

class NEXT_LIBRARY_EXPORT LightSource : public Component {
    A_REGISTER(LightSource, Component, Componets)

    A_PROPERTIES(
        A_PROPERTY(bool, Cast_shadows, LightSource::castShadows, LightSource::setCastShadows),
        A_PROPERTY(double, Brightness, LightSource::brightness, LightSource::setBrightness),
        A_PROPERTY(double, Radius, LightSource::radius, LightSource::setRadius)
    )

public:
    /*! \enum LightTypes */
    enum LightTypes {
        DIRECT      = 1,
        POINT       = 2,
        SPOT        = 3
    };

    LightSource                 ();

    bool                        castShadows             () const;
    void                        setCastShadows          (bool shadows);

    double                      brightness              () const;
    void                        setBrightness           (const double brightness);

    double                      radius                  () const;
    void                        setRadius               (const double radius);

protected:
    /// Type of the source.
    LightTypes                  m_Type;
    /// Cast shadows
    bool                        m_Shadows;
    /// Brightness
    double                      m_Brightness;
    /// Radius (for LIGHT_POINT only)
    double                      m_Radius;
};

#endif // LIGHTSOURCE_H
