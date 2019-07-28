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
        A_PROPERTY(bool,    Cast_shadows,   BaseLight::castShadows, BaseLight::setCastShadows),
        A_PROPERTY(float,   Brightness,     BaseLight::brightness, BaseLight::setBrightness),
        A_PROPERTY(Color,   Color,          BaseLight::color, BaseLight::setColor),
        A_PROPERTY(float,   Bias,           BaseLight::bias, BaseLight::setBias)
    )

public:
    BaseLight ();
    ~BaseLight ();

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
