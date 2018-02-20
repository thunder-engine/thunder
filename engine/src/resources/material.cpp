#include "resources/material.h"
#include "resources/texture.h"

#define PARAMS      "Params"
#define TEXTURES    "Textures"

MaterialInstance::MaterialInstance(Material *material) :
        m_pMaterial(material) {

}

Material *MaterialInstance::material() const {
   return m_pMaterial;
}

void MaterialInstance::setFloat(const char *name, float value) {

}
void MaterialInstance::setVector2(const char *name, const Vector2 &value) {

}
void MaterialInstance::setVector3(const char *name, const Vector3 &value) {

}
void MaterialInstance::setVector4(const char *name, const Vector4 &value) {

}
void MaterialInstance::setMatrix4(const char *name, const Vector4 &value) {

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

void Material::loadUserData(const AVariantMap &data) {
    clear();
    {
        auto it = data.find(PARAMS);
        if(it != data.end()) {
            AVariantList list = (*it).second.value<AVariantList>();
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

bool Material::overrideTexture(const string &name, const Texture *texture) {
    if(texture) {
        auto it = m_Textures.find(name);
        if(it != m_Textures.end()) {
            it->second  = texture;
            return true;
        }
    }
    return false;
}

const Texture *Material::texture(const string &name) {
    auto it = m_Textures.find(name);
    if(it != m_Textures.end()) {
        return it->second;
    }
    return nullptr;
}

MaterialInstance *Material::createInstance() {
    return new MaterialInstance(this);
}
