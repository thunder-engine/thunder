#include "baselight.h"

#include "components/actor.h"
#include "components/transform.h"

#include "systems/rendersystem.h"

#include "material.h"
#include "mesh.h"

namespace {
const char *uniParams  = "uni.params";
const char *uniShadows = "uni.shadows";
const char *uniColor   = "uni.color";
const char *uniBias    = "uni.bias";
};

/*!
    \class BaseLight
    \brief Base class for every light source.
    \inmodule Engine

    \note This class must be a superclass only and shouldn't be created manually.
*/

BaseLight::BaseLight() :
    m_shadows(0.0f),
    m_bias(0.001f),
    m_params(1.0f, 1.0f, 0.5f, 1.0f),
    m_color(1.0f),
    m_materialInstance(nullptr) {

}

BaseLight::~BaseLight() {
    static_cast<RenderSystem *>(system())->removeLight(this);
}

/*!
    Returns true if the light source can cast shadows; otherwise returns false.
*/
bool BaseLight::castShadows() const {
    return (m_shadows == 1.0f);
}
/*!
    Enables or disables cast \a shadows ability for the light source.
*/
void BaseLight::setCastShadows(const bool shadows) {
    m_shadows = (shadows) ? 1.0f : 0.0f;
    if(m_materialInstance) {
        m_materialInstance->setFloat(uniShadows, &m_shadows);
    }
}
/*!
    Returns a brightness of emitting light.
*/
float BaseLight::brightness() const {
    return m_params.x;
}
/*!
    Changes a \a brightness of emitting light.
*/
void BaseLight::setBrightness(const float brightness) {
    m_params.x = brightness;
    if(m_materialInstance) {
        m_materialInstance->setVector4(uniParams, &m_params);
    }
}
/*!
    Returns a color of emitting light.
*/
Vector4 BaseLight::color() const {
    return m_color;
}
/*!
    Changes a \a color of emitting light.
*/
void BaseLight::setColor(const Vector4 color) {
    m_color = color;
    if(m_materialInstance) {
        m_materialInstance->setVector4(uniColor, &m_color);
    }
}
/*!
    Returns shadow map bias value.
*/
Vector4 BaseLight::bias() const {
    return m_bias;
}
/*!
    Changes shadow map \a bias value.
    You can use this value to mitigate the shadow map acne effect.
*/
void BaseLight::setBias(const Vector4 bias) {
    m_bias = bias;
    if(m_materialInstance) {
        m_materialInstance->setVector4(uniBias, &m_bias);
    }
}

int BaseLight::lightType() const {
    return Invalid;
}

/*!
    \internal
*/
MaterialInstance *BaseLight::material() const {
    return m_materialInstance;
}

AABBox BaseLight::bound() const {
    return AABBox(0.0f, -1.0f);
}
/*!
    \internal
*/
void BaseLight::setMaterial(MaterialInstance *instance) {
    m_materialInstance = instance;
    if(m_materialInstance) {
        m_materialInstance->setVector4(uniBias, &m_bias);
        m_materialInstance->setVector4(uniParams, &m_params);
        m_materialInstance->setVector4(uniColor, &m_color);
        m_materialInstance->setFloat(uniShadows, &m_shadows);
    }
}
/*!
    \internal
*/
Vector4 BaseLight::params() const {
    return m_params;
}
/*!
    \internal
*/
void BaseLight::setParams(Vector4 &params) {
    m_params = params;
    if(m_materialInstance) {
        m_materialInstance->setVector4(uniParams, &m_params);
    }
}
/*!
    \internal
*/
void BaseLight::setSystem(ObjectSystem *system) {
    Object::setSystem(system);

    RenderSystem *render = static_cast<RenderSystem *>(system);
    render->addLight(this);
}

Vector4 BaseLight::gizmoColor() {
    return Vector4(1.0f, 1.0f, 0.5f, 1.0f);
}
