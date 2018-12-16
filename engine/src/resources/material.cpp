#include "resources/material.h"
#include "resources/texture.h"

#define PROPERTIES  "Properties"
#define TEXTURES    "Textures"

MaterialInstance::MaterialInstance(Material *material) :
        m_pMaterial(material) {

}

MaterialInstance::~MaterialInstance() {
    m_Info.clear();
}

Material *MaterialInstance::material() const {
    return m_pMaterial;
}

const Texture *MaterialInstance::texture(const char *name) {
    const Texture *result = nullptr;
    auto it = m_Info.find(name);
    if(it != m_Info.end()) {
        result  = static_cast<const Texture *>((*it).second.ptr);
    }
    return result;
}

MaterialInstance::InfoMap MaterialInstance::params() const {
    return m_Info;
}

void MaterialInstance::setInteger(const char *name, const int32_t *value, uint32_t count) {
    Info info;
    info.count      = count;
    info.ptr        = value;
    info.type       = MetaType::INTEGER;

    m_Info[name]    = info;
}
void MaterialInstance::setFloat(const char *name, const float *value, uint32_t count) {
    Info info;
    info.count      = count;
    info.ptr        = value;
    info.type       = MetaType::FLOAT;

    m_Info[name]    = info;
}
void MaterialInstance::setVector2(const char *name, const Vector2 *value, uint32_t count) {
    Info info;
    info.count      = count;
    info.ptr        = value;
    info.type       = MetaType::VECTOR2;

    m_Info[name]    = info;
}
void MaterialInstance::setVector3(const char *name, const Vector3 *value, uint32_t count) {
    Info info;
    info.count      = count;
    info.ptr        = value;
    info.type       = MetaType::VECTOR3;

    m_Info[name]    = info;
}
void MaterialInstance::setVector4(const char *name, const Vector4 *value, uint32_t count) {
    Info info;
    info.count      = count;
    info.ptr        = value;
    info.type       = MetaType::VECTOR4;

    m_Info[name]    = info;
}
void MaterialInstance::setMatrix4(const char *name, const Matrix4 *value, uint32_t count) {
    Info info;
    info.count      = count;
    info.ptr        = value;
    info.type       = MetaType::MATRIX4;

    m_Info[name]    = info;
}

void MaterialInstance::setTexture(const char *name, const Texture *value, uint32_t count) {
    Info info;
    info.count      = count;
    info.ptr        = value;
    info.type       = 0;

    m_Info[name]    = info;
}

Material::Material() :
        m_BlendMode(Opaque),
        m_LightModel(Unlit),
        m_MaterialType(Surface),
        m_DoubleSided(true),
        m_DepthTest(true),
        m_Surfaces(1) {
    clear();
}

Material::~Material() {
    clear();
}

void Material::loadUserData(const VariantMap &data) {
    clear();
    {
        auto it = data.find(PROPERTIES);
        if(it != data.end()) {
            VariantList list    = (*it).second.value<VariantList>();
            auto i  = list.begin();
            m_MaterialType  = static_cast<MaterialType>((*i).toInt());
            i++;
            m_DoubleSided   = (*i).toBool();
            i++;
            m_Surfaces      = (*i).toInt();
            i++;
            m_BlendMode     = static_cast<BlendType>((*i).toInt());
            i++;
            m_LightModel    = static_cast<LightModelType>((*i).toInt());
            i++;
            m_DepthTest     = (*i).toBool();
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
    return static_cast<LightModelType>(m_LightModel);
}

Material::BlendType Material::blendMode() const {
    return static_cast<BlendType>(m_BlendMode);
}

bool Material::isDoubleSided() const {
    return m_DoubleSided;
}

void Material::setDoubleSided(bool flag) {
    m_DoubleSided   = flag;
}

bool Material::isDepthTest() const {
    return m_DepthTest;
}

void Material::setDepthTest(bool flag) {
    m_DepthTest = flag;
}

void Material::setTexture(const string &name, const Texture *texture) {
    m_Textures[name]    = texture;
}

uint8_t Material::surfaces() const {
    return m_Surfaces;
}

MaterialInstance *Material::createInstance() {
    return new MaterialInstance(this);
}
