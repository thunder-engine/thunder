#ifndef BASELIGHT_H
#define BASELIGHT_H

#include "renderable.h"

#include <amath.h>

class MaterialInstance;

class ENGINE_EXPORT BaseLight : public NativeBehaviour {
    A_OBJECT(BaseLight, NativeBehaviour, General)

    A_PROPERTIES(
        A_PROPERTY(bool, castShadows, BaseLight::castShadows, BaseLight::setCastShadows),
        A_PROPERTY(float, brightness, BaseLight::brightness, BaseLight::setBrightness),
        A_PROPERTYEX(Vector4, color, BaseLight::color, BaseLight::setColor, "editor=Color")
    )
    A_NOMETHODS()

public:
    enum LightType {
        Invalid,
        DirectLight,
        AreaLight,
        PointLight,
        SpotLight
    };

public:
    BaseLight();
    ~BaseLight();

    bool castShadows() const;
    void setCastShadows(const bool shadows);

    float brightness() const;
    void setBrightness(const float brightness);

    Vector4 color() const;
    void setColor(const Vector4 color);

    virtual int lightType() const;

    MaterialInstance *material() const;

    virtual AABBox bound() const;

protected:
    void setMaterial(MaterialInstance *instance);

    Vector4 params() const;
    void setParams(Vector4 &params);

    Vector4 gizmoColor() const;

private:
    void setSystem(ObjectSystem *system) override;

private:
    Vector4 m_params;

    Vector4 m_color;

    MaterialInstance *m_materialInstance;

    bool m_shadows;

};

#endif // BASELIGHT_H
