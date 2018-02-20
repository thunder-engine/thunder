#ifndef DIRECTLIGHT_H
#define DIRECTLIGHT_H

#include "component.h"

#include <amath.h>

class NEXT_LIBRARY_EXPORT DirectLight : public Component {
    A_REGISTER(DirectLight, Component, Components)

    A_PROPERTIES(
        A_PROPERTY(bool, Cast_shadows, DirectLight::castShadows, DirectLight::setCastShadows),
        A_PROPERTY(double, Brightness, DirectLight::brightness, DirectLight::setBrightness),
        A_PROPERTY(Color, Color, DirectLight::color, DirectLight::setColor)
    )

public:
    DirectLight                 ();

    bool                        castShadows             () const;
    void                        setCastShadows          (bool shadows);

    double                      brightness              () const;
    void                        setBrightness           (const double brightness);

    Vector4                     color                   () const;
    void                        setColor                (const Vector4 &color);

protected:
    bool                        m_Shadows;

    double                      m_Brightness;

    Vector4                     m_Color;
};

#endif // DIRECTLIGHT_H
