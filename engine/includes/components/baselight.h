#ifndef BASELIGHT_H
#define BASELIGHT_H

#include "renderable.h"

#include <amath.h>

class Mesh;
class MaterialInstance;
class BaseLightPrivate;
class Pipeline;

class Camera;

#define SM_RESOLUTION_DEFAULT 2048

#define SHADOW_MAP  "shadowMap"

class NEXT_LIBRARY_EXPORT BaseLight : public Renderable {
    A_REGISTER(BaseLight, Renderable, General)

    A_PROPERTIES(
        A_PROPERTY(bool, castShadows, BaseLight::castShadows, BaseLight::setCastShadows),
        A_PROPERTY(float, brightness, BaseLight::brightness, BaseLight::setBrightness),
        A_PROPERTYEX(Vector4, color, BaseLight::color, BaseLight::setColor, "editor=Color"),
        A_PROPERTY(float, bias, BaseLight::bias, BaseLight::setBias)
    )
    A_NOMETHODS()

public:
    BaseLight ();
    ~BaseLight () override;

    virtual void shadowsUpdate(const Camera &camera, Pipeline *pipeline, ObjectList &components);

    bool castShadows () const;
    void setCastShadows (const bool shadows);

    float brightness () const;
    void setBrightness (const float brightness);

    Vector4 color () const;
    void setColor (const Vector4 &color);

    float bias () const;
    void setBias (const float bias);

protected:
    MaterialInstance *material() const;
    void setMaterial(MaterialInstance *instance);

    Mesh *shape() const;
    void setShape(Mesh *shape);

    Vector4 params() const;
    void setParams(Vector4 &params);

private:
    BaseLightPrivate *p_ptr;

};

#endif // BASELIGHT_H
