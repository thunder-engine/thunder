#include "baselight.h"

#include "components/actor.h"
#include "components/transform.h"

#include "material.h"
#include "mesh.h"

namespace {
const char *uni_params  = "uni.params";
const char *uni_shadows = "uni.shadows";
const char *uni_color   = "uni.color";
const char *uni_bias    = "uni.bias";
};

class BaseLightPrivate {
public:
    BaseLightPrivate() :
            m_Shadows(0.0f),
            m_Bias(0.001f),
            m_Params(1.0f, 1.0f, 0.5f, 1.0f),
            m_Color(1.0f),
            m_pShape(nullptr),
            m_pMaterialInstance(nullptr) {

    }

    float m_Shadows;

    Vector4 m_Bias;

    Vector4 m_Params;

    Vector4 m_Color;

    Mesh *m_pShape;

    MaterialInstance *m_pMaterialInstance;
};
/*!
    \class BaseLight
    \brief Base class for every light source.
    \inmodule Engine

    \note This class must be a superclass only and shouldn't be created manually.
*/

BaseLight::BaseLight() :
        p_ptr(new BaseLightPrivate) {

}

BaseLight::~BaseLight() {
    delete p_ptr;
    p_ptr = nullptr;
}

/*!
    Updates the shadowmaps for the particular lightsource.

    \internal
*/
void BaseLight::shadowsUpdate(const Camera &camera, PipelineContext *context, RenderList &components) {
    A_UNUSED(camera);
    A_UNUSED(context);
    A_UNUSED(components);
}

/*!
    Returns true if the light source can cast shadows; otherwise returns false.
*/
bool BaseLight::castShadows() const {
    return (p_ptr->m_Shadows == 1.0f);
}
/*!
    Enables or disables cast \a shadows ability for the light source.
*/
void BaseLight::setCastShadows(const bool shadows) {
    p_ptr->m_Shadows = (shadows) ? 1.0f : 0.0f;
    if(p_ptr->m_pMaterialInstance) {
        p_ptr->m_pMaterialInstance->setFloat(uni_shadows, &p_ptr->m_Shadows);
    }
}
/*!
    Returns a brightness of emitting light.
*/
float BaseLight::brightness() const {
    return p_ptr->m_Params.x;
}
/*!
    Changes a \a brightness of emitting light.
*/
void BaseLight::setBrightness(const float brightness) {
    p_ptr->m_Params.x = brightness;
    if(p_ptr->m_pMaterialInstance) {
        p_ptr->m_pMaterialInstance->setVector4(uni_params, &p_ptr->m_Params);
    }
}
/*!
    Returns a color of emitting light.
*/
Vector4 &BaseLight::color() const {
    return p_ptr->m_Color;
}
/*!
    Changes a \a color of emitting light.
*/
void BaseLight::setColor(const Vector4 color) {
    p_ptr->m_Color = color;
    if(p_ptr->m_pMaterialInstance) {
        p_ptr->m_pMaterialInstance->setVector4(uni_color, &p_ptr->m_Color);
    }
}
/*!
    Returns shadow map bias value.
*/
Vector4 &BaseLight::bias() const {
    return p_ptr->m_Bias;
}
/*!
    Changes shadow map \a bias value.
    You can use this value to mitigate the shadow map acne effect.
*/
void BaseLight::setBias(const Vector4 bias) {
    p_ptr->m_Bias = bias;
    if(p_ptr->m_pMaterialInstance) {
        p_ptr->m_pMaterialInstance->setVector4(uni_bias, &p_ptr->m_Bias);
    }
}
/*!
    \internal
*/
MaterialInstance *BaseLight::material() const {
    return p_ptr->m_pMaterialInstance;
}
/*!
    \internal
*/
void BaseLight::setMaterial(MaterialInstance *instance) {
    p_ptr->m_pMaterialInstance = instance;
    if(p_ptr->m_pMaterialInstance) {
        p_ptr->m_pMaterialInstance->setVector4(uni_bias, &p_ptr->m_Bias);
        p_ptr->m_pMaterialInstance->setVector4(uni_params, &p_ptr->m_Params);
        p_ptr->m_pMaterialInstance->setVector4(uni_color, &p_ptr->m_Color);
        p_ptr->m_pMaterialInstance->setFloat(uni_shadows, &p_ptr->m_Shadows);
    }
}
/*!
    \internal
*/
Mesh *BaseLight::shape() const {
    return p_ptr->m_pShape;
}
/*!
    \internal
*/
void BaseLight::setShape(Mesh *shape) {
    p_ptr->m_pShape = shape;
}
/*!
    \internal
*/
Vector4 BaseLight::params() const {
    return p_ptr->m_Params;
}
/*!
    \internal
*/
void BaseLight::setParams(Vector4 &params) {
    p_ptr->m_Params = params;
    if(p_ptr->m_pMaterialInstance) {
        p_ptr->m_pMaterialInstance->setVector4(uni_params, &p_ptr->m_Params);
    }
}

bool BaseLight::isLight() const {
    return true;
}
