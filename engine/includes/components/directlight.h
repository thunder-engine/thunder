#ifndef DIRECTLIGHT_H
#define DIRECTLIGHT_H

#include "component.h"

#include <amath.h>

class Mesh;
class Material;
class MaterialInstance;

class NEXT_LIBRARY_EXPORT DirectLight : public Component {
    A_REGISTER(DirectLight, Component, Components);

    A_PROPERTIES(
        A_PROPERTY(bool,    Cast_shadows,   DirectLight::castShadows, DirectLight::setCastShadows),
        A_PROPERTY(float,   Brightness,     DirectLight::brightness, DirectLight::setBrightness),
        A_PROPERTY(Color,   Color,          DirectLight::color, DirectLight::setColor)
    );

public:
    DirectLight                 ();

    ~DirectLight                ();

    void                        draw                (ICommandBuffer &buffer, int8_t layer);

    bool                        castShadows             () const;
    void                        setCastShadows          (bool shadows);

    float                       brightness              () const;
    void                        setBrightness           (const float brightness);

    Vector4                     color                   () const;
    void                        setColor                (const Vector4 &color);

protected:
    float                       m_Shadows;

    float                       m_Brightness;

    float                       m_Bias;

    Vector4                     m_Color;

    /// Number of levels of detail shadows. 0 - if shadows are disabled.
    uint8_t                     m_LODCount;

    /// Light source are using the Cascaded Shadow Maps.
    bool                        m_CSM;
    /// Light source matrix array.
    Matrix4                    *m_pMatrix;
    Vector4                    *m_pTiles;
    /// Distance of cutting for each level of LOD.
    Vector4                     m_Distance;

    Vector4                     m_NormalizedDistance;

    Mesh                       *m_pPlane;

    Material                   *m_pMaterial;
    MaterialInstance           *m_pMaterialInstance;
};

#endif // DIRECTLIGHT_H
