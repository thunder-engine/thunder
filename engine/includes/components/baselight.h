#ifndef BASELIGHT_H
#define BASELIGHT_H

#include "renderable.h"

#include <amath.h>

class MaterialInstance;

class ENGINE_EXPORT BaseLight : public Renderable {
    A_REGISTER(BaseLight, Renderable, General)

    A_PROPERTIES(
        A_PROPERTY(bool, castShadows, BaseLight::castShadows, BaseLight::setCastShadows),
        A_PROPERTY(float, brightness, BaseLight::brightness, BaseLight::setBrightness),
        A_PROPERTYEX(Vector4, color, BaseLight::color, BaseLight::setColor, "editor=Color"),
        A_PROPERTY(Vector4, bias, BaseLight::bias, BaseLight::setBias)
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

    bool castShadows() const;
    void setCastShadows(const bool shadows);

    float brightness() const;
    void setBrightness(const float brightness);

    Vector4 color() const;
    void setColor(const Vector4 color);

    Vector4 bias() const;
    void setBias(const Vector4 bias);

    virtual int lightType() const;

    MaterialInstance *material() const;

protected:
    void setMaterial(MaterialInstance *instance);

    Vector4 params() const;
    void setParams(Vector4 &params);

private:
    bool isLight() const override;

private:
    float m_shadows;

    Vector4 m_bias;

    Vector4 m_params;

    Vector4 m_color;

    MaterialInstance *m_materialInstance;

};

#endif // BASELIGHT_H
