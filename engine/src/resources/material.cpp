#include "resources/material.h"
#include "resources/texture.h"

#define PROPERTIES  "Properties"
#define TEXTURES    "Textures"
#define UNIFORMS    "Uniforms"

MaterialInstance::MaterialInstance(Material *material) :
        m_pMaterial(material),
        m_SurfaceType(0) {

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
        result = static_cast<Texture *>((*it).second.ptr);
    }
    return result;
}

MaterialInstance::InfoMap &MaterialInstance::params() {
    return m_Info;
}

void MaterialInstance::setInteger(const char *name, int32_t *value, int32_t count) {
    Info info;
    info.count = count;
    info.ptr = value;
    info.type = MetaType::INTEGER;

    m_Info[name] = info;
}
void MaterialInstance::setFloat(const char *name, float *value, int32_t count) {
    Info info;
    info.count = count;
    info.ptr = value;
    info.type = MetaType::FLOAT;

    m_Info[name] = info;
}
void MaterialInstance::setVector2(const char *name, Vector2 *value, int32_t count) {
    Info info;
    info.count = count;
    info.ptr = value;
    info.type = MetaType::VECTOR2;

    m_Info[name] = info;
}
void MaterialInstance::setVector3(const char *name, Vector3 *value, int32_t count) {
    Info info;
    info.count = count;
    info.ptr = value;
    info.type = MetaType::VECTOR3;

    m_Info[name] = info;
}
void MaterialInstance::setVector4(const char *name, Vector4 *value, int32_t count) {
    Info info;
    info.count = count;
    info.ptr = value;
    info.type = MetaType::VECTOR4;

    m_Info[name] = info;
}
void MaterialInstance::setMatrix4(const char *name, Matrix4 *value, int32_t count) {
    Info info;
    info.count = count;
    info.ptr = value;
    info.type = MetaType::MATRIX4;

    m_Info[name] = info;
}

void MaterialInstance::setTexture(const char *name, Texture *value, int32_t count) {
    Info info;
    info.count = count;
    info.ptr = value;
    info.type = 0;

    m_Info[name] = info;
}

uint16_t MaterialInstance::surfaceType() const {
    return m_SurfaceType;
}

void MaterialInstance::setSurfaceType(uint16_t type) {
    m_SurfaceType = type;
}

/*!
    \class Material
    \brief A Material is a resource which can be applied to a Mesh to control the visual look of the scene.
    \inmodule Resource
*/

Material::Material() :
        m_BlendMode(Opaque),
        m_LightModel(Unlit),
        m_MaterialType(Surface),
        m_DoubleSided(true),
        m_DepthTest(true),
        m_DepthWrite(true),
        m_Surfaces(1) {
    clear();
}

Material::~Material() {
    clear();
}
/*!
    Removes all attached textures from the material.
*/
void Material::clear() {
    m_Textures.clear();
}
/*!
    Returns current material type.
    For more detalse please refer to Material::MaterialType enum.
*/
int Material::materialType() const {
    return m_MaterialType;
}
/*!
    Sets new material \a type.
    For more detalse please refer to Material::MaterialType enum.
*/
void Material::setMaterialType(int type) {
    m_MaterialType = type;
}
/*!
    Returns current light model for the material.
    For more detalse please refer to Material::LightModelType enum.
*/
int Material::lightModel() const {
    return m_LightModel;
}
/*!
    Sets a new light \a model for the material.
    For more detalse please refer to Material::LightModelType enum.
*/
void Material::setLightModel(int model) {
    m_LightModel = model;
}
/*!
    Returns current blend mode for the material.
    For more detalse please refer to Material::BlendType enum.
*/
int Material::blendMode() const {
    return m_BlendMode;
}
/*!
    Sets a new blend \a mode for the material.
    For more detalse please refer to Material::BlendType enum.
*/
void Material::setBlendMode(int mode) {
    m_BlendMode = mode;
}
/*!
    Returns true if mas marked as double-sided; otherwise returns false.
*/
bool Material::doubleSided() const {
    return m_DoubleSided;
}
/*!
    Enables or disables the double-sided \a flag for the material.
*/
void Material::setDoubleSided(bool flag) {
    m_DoubleSided = flag;
}
/*!
    Returns true if depth test was enabled; otherwise returns false.
*/
bool Material::depthTest() const {
    return m_DepthTest;
}
/*!
    Enables or disables a depth \a test for the material.
*/
void Material::setDepthTest(bool test) {
    m_DepthTest = test;
}
/*!
    Sets a \a texture with a given \a name for the material.
*/
void Material::setTexture(const string &name, Texture *texture) {
    m_Textures[name] = texture;
}
/*!
    \internal
*/
void Material::loadUserData(const VariantMap &data) {
    clear();
    {
        auto it = data.find(PROPERTIES);
        if(it != data.end()) {
            VariantList list = (*it).second.value<VariantList>();
            auto i = list.begin();
            m_MaterialType = static_cast<MaterialType>((*i).toInt());
            i++;
            m_DoubleSided = (*i).toBool();
            i++;
            m_Surfaces = (*i).toInt();
            i++;
            m_BlendMode = static_cast<BlendType>((*i).toInt());
            i++;
            m_LightModel = static_cast<LightModelType>((*i).toInt());
            i++;
            m_DepthTest = (*i).toBool();
            i++;
            m_DepthWrite = (*i).toBool();
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
/*!
    Returns a new instance for the material with the provided surface \a type.
*/
MaterialInstance *Material::createInstance(SurfaceType type) {
    A_UNUSED(type);
    MaterialInstance *result = new MaterialInstance(this);
    for(auto &it : m_Uniforms) {
        MaterialInstance::Info info;
        info.count = 1;
        info.ptr = it.second.data();
        info.type = it.second.type();

        result->m_Info[it.first] = info;
    }

    return result;
}
