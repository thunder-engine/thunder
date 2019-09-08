#include "resources/material.h"
#include "resources/texture.h"

#define PROPERTIES  "Properties"
#define TEXTURES    "Textures"
#define UNIFORMS    "Uniforms"

MaterialInstance::MaterialInstance(Material *material) :
        m_pMaterial(material) {

}

MaterialInstance::~MaterialInstance() {
    m_Info.clear();
}

Material *MaterialInstance::material() const {
    return m_pMaterial;
}

Texture *MaterialInstance::texture(const char *name) {
    Texture *result = nullptr;
    auto it = m_Info.find(name);
    if(it != m_Info.end()) {
        result  = static_cast<Texture *>((*it).second.ptr);
    }
    return result;
}

MaterialInstance::InfoMap &MaterialInstance::params() {
    return m_Info;
}

void MaterialInstance::setInteger(const char *name, int32_t *value, int32_t count) {
    Info info;
    info.count      = count;
    info.ptr        = value;
    info.type       = MetaType::INTEGER;

    m_Info[name]    = info;
}
void MaterialInstance::setFloat(const char *name, float *value, int32_t count) {
    Info info;
    info.count      = count;
    info.ptr        = value;
    info.type       = MetaType::FLOAT;

    m_Info[name]    = info;
}
void MaterialInstance::setVector2(const char *name, Vector2 *value, int32_t count) {
    Info info;
    info.count      = count;
    info.ptr        = value;
    info.type       = MetaType::VECTOR2;

    m_Info[name]    = info;
}
void MaterialInstance::setVector3(const char *name, Vector3 *value, int32_t count) {
    Info info;
    info.count      = count;
    info.ptr        = value;
    info.type       = MetaType::VECTOR3;

    m_Info[name]    = info;
}
void MaterialInstance::setVector4(const char *name, Vector4 *value, int32_t count) {
    Info info;
    info.count      = count;
    info.ptr        = value;
    info.type       = MetaType::VECTOR4;

    m_Info[name]    = info;
}
void MaterialInstance::setMatrix4(const char *name, Matrix4 *value, int32_t count) {
    Info info;
    info.count      = count;
    info.ptr        = value;
    info.type       = MetaType::MATRIX4;

    m_Info[name]    = info;
}

void MaterialInstance::setTexture(const char *name, Texture *value, int32_t count) {
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
                string path = t.second.toString();
                if(!path.empty()) {
                    m_Textures[t.first] = Engine::loadResource<Texture>(path);
                } else {
                    m_Textures[t.first] = nullptr;
                }
            }
        }
    }
    {
        auto it = data.find(UNIFORMS);
        if(it != data.end()) {
            for(auto u : (*it).second.toMap()) {
                m_Uniforms[u.first] = u.second;
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

void Material::setTexture(const string &name, Texture *texture) {
    m_Textures[name] = texture;
}

int32_t Material::surfaces() const {
    return m_Surfaces;
}

MaterialInstance *Material::createInstance() {
    MaterialInstance *result = new MaterialInstance(this);
    for(auto &it : m_Uniforms) {
        MaterialInstance::Info info;
        info.count = 1;
        info.ptr   = it.second.data();
        info.type  = it.second.type();

        result->m_Info[it.first] = info;
    }

    return result;
}
