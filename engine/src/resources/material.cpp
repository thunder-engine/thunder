#include "resources/material.h"
#include "resources/texture.h"

#define PARAMS      "Params"
#define TEXTURES    "Textures"

MaterialInstance::MaterialInstance(Material *material) :
        m_pMaterial(material) {

}

MaterialInstance::~MaterialInstance() {
    m_Textures.clear();
    m_Uniforms.clear();
}

Material *MaterialInstance::material() const {
   return m_pMaterial;
}

const Texture *MaterialInstance::texture(const char *name) {
    const Texture *result = nullptr;
    auto it = m_Textures.find(name);
    if(it != m_Textures.end()) {
        result  = (*it).second;
    }
    return result;
}

void MaterialInstance::setFloat(const char *name, float value) {
    m_Uniforms[name]    = value;
}
void MaterialInstance::setVector2(const char *name, const Vector2 &value) {
    m_Uniforms[name]    = value;
}
void MaterialInstance::setVector3(const char *name, const Vector3 &value) {
    m_Uniforms[name]    = value;
}
void MaterialInstance::setVector4(const char *name, const Vector4 &value) {
    m_Uniforms[name]    = value;
}
void MaterialInstance::setMatrix4(const char *name, const Vector4 &value) {
    m_Uniforms[name]    = value;
}

void MaterialInstance::setTexture(const char *name, const Texture *value) {
    m_Textures[name]    = value;
}

Material::Material() :
        m_BlendMode(Opaque),
        m_LightModel(Unlit),
        m_MaterialType(Surface),
        m_DoubleSided(true),
        m_Tangent(false),
        m_Surfaces(1) {

}

Material::~Material() {
    clear();
}

void Material::loadUserData(const VariantMap &data) {
    clear();
    {
        auto it = data.find(PARAMS);
        if(it != data.end()) {
            VariantList list    = (*it).second.value<VariantList>();
            auto i  = list.begin();
            m_MaterialType  = (MaterialType)(*i).toInt();
            i++;
            m_DoubleSided   = (*i).toBool();
            i++;
            m_Tangent       = (*i).toBool();
            i++;
            m_Surfaces      = (*i).toInt();
            i++;
            m_BlendMode     = (BlendType)(*i).toInt();
            i++;
            m_LightModel    = (LightModelType)(*i).toInt();
            i++;
        }
    }
    {
        auto it = data.find(TEXTURES);
        if(it != data.end()) {
            for(auto t : (*it).second.toMap()) {
                m_Textures[t.first] = Engine::loadResource<Texture>(t.second.toString());
            }
        }
    }
}

Material::MaterialType Material::materialType() const {
    return m_MaterialType;
}

void Material::clear() {
    m_Textures.clear();
}

Material::LightModelType Material::lightModel() const {
    return (LightModelType)m_LightModel;
}

Material::BlendType Material::blendMode() const {
    return (BlendType)m_BlendMode;
}

bool Material::isDoubleSided() const {
    return m_DoubleSided;
}

void Material::setDoubleSided(bool flag) {
    m_DoubleSided   = flag;
}

uint8_t Material::surfaces() const {
    return m_Surfaces;
}

MaterialInstance *Material::createInstance() {
    return new MaterialInstance(this);
}
