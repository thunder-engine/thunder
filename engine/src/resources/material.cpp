#include "resources/material.h"
#include "resources/texture.h"

#include <cstring>

#define PROPERTIES  "Properties"
#define TEXTURES    "Textures"
#define UNIFORMS    "Uniforms"
#define ATTRIBUTES  "Attributes"

MaterialInstance::MaterialInstance(Material *material) :
        m_material(material),
        m_uniformBuffer(nullptr),
        m_surfaceType(0),
        m_uniformDirty(true) {

    if(m_material->m_uniformSize > 0) {
        m_uniformBuffer = new uint8_t[m_material->m_uniformSize];
    }
}

MaterialInstance::~MaterialInstance() {
    delete []m_uniformBuffer;
}

Material *MaterialInstance::material() const {
    return m_material;
}

Texture *MaterialInstance::texture(const char *name) {
    auto it = m_textureOverride.find(name);
    if(it != m_textureOverride.end()) {
        return (*it).second;
    }
    return nullptr;
}

void MaterialInstance::setBool(const char *name, const bool *value, int32_t count) {
    if(count > 1) {
        VariantList list;
        for(int32_t i = 0; i < count; i++) {
            list.push_back(value[i]);
        }
        m_paramOverride[name] = list;
    } else {
        m_paramOverride[name] = *value;
    }

    setBufferValue(name, value);
}

void MaterialInstance::setInteger(const char *name, const int32_t *value, int32_t count) {
    if(count > 1) {
        VariantList list;
        for(int32_t i = 0; i < count; i++) {
            list.push_back(value[i]);
        }
        m_paramOverride[name] = list;
    } else {
        m_paramOverride[name] = *value;
    }

    setBufferValue(name, value);
}

void MaterialInstance::setFloat(const char *name, const float *value, int32_t count) {
    if(count > 1) {
        VariantList list;
        for(int32_t i = 0; i < count; i++) {
            list.push_back(value[i]);
        }
        m_paramOverride[name] = list;
    } else {
        m_paramOverride[name] = *value;
    }

    setBufferValue(name, value);
}

void MaterialInstance::setVector2(const char *name, const Vector2 *value, int32_t count) {
    if(count > 1) {
        VariantList list;
        for(int32_t i = 0; i < count; i++) {
            list.push_back(value[i]);
        }
        m_paramOverride[name] = list;
    } else {
        m_paramOverride[name] = *value;
    }

    setBufferValue(name, value);
}

void MaterialInstance::setVector3(const char *name, const Vector3 *value, int32_t count) {
    if(count > 1) {
        VariantList list;
        for(int32_t i = 0; i < count; i++) {
            list.push_back(value[i]);
        }
        m_paramOverride[name] = list;
    } else {
        m_paramOverride[name] = *value;
    }

    setBufferValue(name, value);
}

void MaterialInstance::setVector4(const char *name, const Vector4 *value, int32_t count) {
    if(count > 1) {
        VariantList list;
        for(int32_t i = 0; i < count; i++) {
            list.push_back(value[i]);
        }
        m_paramOverride[name] = list;
    } else {
        m_paramOverride[name] = *value;
    }

    setBufferValue(name, value);
}

void MaterialInstance::setMatrix4(const char *name, const Matrix4 *value, int32_t count) {
    if(count > 1) {
        VariantList list;
        for(int32_t i = 0; i < count; i++) {
            list.push_back(value[i]);
        }
        m_paramOverride[name] = list;
    } else {
        m_paramOverride[name] = *value;
    }

    setBufferValue(name, value);
}

void MaterialInstance::setBufferValue(const char *name, const void *value) {
    for(auto &it : m_material->m_uniforms) {
        if(it.name == name) {
            if(m_uniformBuffer) {
                memcpy(&m_uniformBuffer[it.offset], value, it.size);
                m_uniformDirty = true;
            }
            break;
        }
    }
}

void MaterialInstance::setTexture(const char *name, Texture *value) {
    m_textureOverride[name] = value;
}

uint32_t MaterialInstance::paramCount() const {
    return m_material->m_uniforms.size();
}

string MaterialInstance::paramName(uint32_t index) const {
    if(index < m_material->m_uniforms.size()) {
        return m_material->m_uniforms[index].name;
    }
    return string();
}

Variant MaterialInstance::paramValue(uint32_t index) const {
    if(index < m_material->m_uniforms.size()) {
        auto it = m_paramOverride.find(m_material->m_uniforms[index].name);
        if(it != m_paramOverride.end()) {
            return it->second;
        }
        return m_material->m_uniforms[index].defaultValue;
    }
    return Variant();
}

uint16_t MaterialInstance::surfaceType() const {
    return m_surfaceType;
}

void MaterialInstance::setSurfaceType(uint16_t type) {
    m_surfaceType = type;
}

/*!
    \class Material
    \brief A Material is a resource which can be applied to a Mesh to control the visual look of the scene.
    \inmodule Resources
*/

Material::Material() :
        m_uniformSize(0),
        m_surfaces(1),
        m_blendMode(Opaque),
        m_lightModel(Unlit),
        m_materialType(Surface),
        m_doubleSided(true),
        m_depthTest(true),
        m_depthWrite(true),
        m_wireframe(false) {

}

Material::~Material() {

}
/*!
    Returns current material type.
    For more detalse please refer to Material::MaterialType enum.
*/
int Material::materialType() const {
    return m_materialType;
}
/*!
    Sets new material \a type.
    For more detalse please refer to Material::MaterialType enum.
*/
void Material::setMaterialType(int type) {
    m_materialType = type;
}
/*!
    Returns current light model for the material.
    For more detalse please refer to Material::LightModelType enum.
*/
int Material::lightModel() const {
    return m_lightModel;
}
/*!
    Sets a new light \a model for the material.
    For more detalse please refer to Material::LightModelType enum.
*/
void Material::setLightModel(int model) {
    m_lightModel = model;
}
/*!
    Returns current blend mode for the material.
    For more detalse please refer to Material::BlendType enum.
*/
int Material::blendMode() const {
    return m_blendMode;
}
/*!
    Sets a new blend \a mode for the material.
    For more detalse please refer to Material::BlendType enum.
*/
void Material::setBlendMode(int mode) {
    m_blendMode = mode;
}
/*!
    Returns true if mas marked as double-sided; otherwise returns false.
*/
bool Material::doubleSided() const {
    return m_doubleSided;
}
/*!
    Enables or disables the double-sided \a flag for the material.
*/
void Material::setDoubleSided(bool flag) {
    m_doubleSided = flag;
}
/*!
    Returns true if depth test was enabled; otherwise returns false.
*/
bool Material::depthTest() const {
    return m_depthTest;
}
/*!
    Enables or disables a depth \a test for the material.
*/
void Material::setDepthTest(bool test) {
    m_depthTest = test;
}
/*!
    Returns true if write opertaion to the depth buffer was enabled; otherwise returns false.
*/
bool Material::depthWrite() const {
    return m_depthWrite;
}
/*!
    Enables or disables a \a write operation to the depth buffer.
*/
void Material::setDepthWrite(bool write) {
    m_depthWrite = write;
}
/*!
    Sets a \a texture with a given \a name for the material.
*/
void Material::setTexture(const string &name, Texture *texture) {
    for(auto &it : m_textures) {
        if(it.name == name) {
            it.texture = texture;
            return;
        }
    }

    TextureItem item;
    item.name = name;
    item.texture = texture;
    item.binding = -1;
    m_textures.push_back(item);
}
/*!
    Returns true if material must be rendered as wireframe.
*/
bool Material::wireframe() const {
    return m_wireframe;
}
/*!
    Enables or disables a \a wireframe mode for the material.
*/
void Material::setWireframe(bool wireframe) {
    m_wireframe = wireframe;
}
/*!
    \internal
*/
void Material::loadUserData(const VariantMap &data) {
    {
        auto it = data.find(PROPERTIES);
        if(it != data.end()) {
            VariantList list = (*it).second.value<VariantList>();
            auto i = list.begin();
            m_materialType = static_cast<MaterialType>((*i).toInt());
            i++;
            m_doubleSided = (*i).toBool();
            i++;
            m_surfaces = (*i).toInt();
            i++;
            m_blendMode = static_cast<BlendType>((*i).toInt());
            i++;
            m_lightModel = static_cast<LightModelType>((*i).toInt());
            i++;
            m_depthTest = (*i).toBool();
            i++;
            m_depthWrite = (*i).toBool();
            i++;
            m_wireframe = (*i).toBool();
        }
    }
    {
        m_textures.clear();
        auto it = data.find(TEXTURES);
        if(it != data.end()) {
            for(auto &t : (*it).second.toList()) {
                VariantList list = t.toList();
                auto f = list.begin();
                string path = (*f).toString();
                TextureItem item;
                item.texture = nullptr;
                if(!path.empty()) {
                    item.texture = Engine::loadResource<Texture>(path);
                }
                ++f;
                item.binding = (*f).toInt();
                ++f;
                item.name = (*f).toString();
                ++f;
                item.flags = (*f).toInt();

                m_textures.push_back(item);
            }
        }
    }
    {
        m_uniformSize = 0;
        m_uniforms.clear();
        auto it = data.find(UNIFORMS);
        if(it != data.end()) {
            size_t offset = 0;
            VariantList uniforms = (*it).second.toList();
            m_uniforms.resize(uniforms.size());
            int i = 0;
            for(auto &u : uniforms) {
                VariantList list = u.toList();
                auto f = list.begin();

                m_uniforms[i].defaultValue = (*f);
                ++f;
                m_uniforms[i].size = (*f).toInt();
                ++f;
                m_uniforms[i].name = (*f).toString();

                m_uniforms[i].offset = offset;
                offset += m_uniforms[i].size;

                i++;
            }
            m_uniformSize = offset;
        }
    }
    {
        m_attributes.clear();
        auto it = data.find(ATTRIBUTES);
        if(it != data.end()) {
            VariantList attributes = (*it).second.toList();
            m_attributes.resize(attributes.size());
            int i = 0;
            for(auto &a : attributes) {
                VariantList list = a.toList();
                auto f = list.begin();

                m_attributes[i].format = (*f).toInt();
                ++f;
                m_attributes[i].location = (*f).toInt();

                i++;
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

    initInstance(result);

    return result;
}
/*!
    \internal
*/
void Material::switchState(ResourceState state) {
    setState(state);
}
/*!
    \internal
*/
bool Material::isUnloadable() {
    return true;
}
/*!
    \internal
*/
void Material::initInstance(MaterialInstance *instance) {
    if(instance) {
        for(auto &it : m_uniforms) {
            switch(it.defaultValue.type()) {
            case MetaType::INTEGER: {
                int32_t value = it.defaultValue.toInt();
                instance->setInteger(it.name.c_str(), &value);
            } break;
            case MetaType::FLOAT: {
                float value = it.defaultValue.toFloat();
                instance->setFloat(it.name.c_str(), &value);
            } break;
            case MetaType::VECTOR2: {
                Vector2 value = it.defaultValue.toVector2();
                instance->setVector2(it.name.c_str(), &value);
            } break;
            case MetaType::VECTOR3: {
                Vector3 value = it.defaultValue.toVector3();
                instance->setVector3(it.name.c_str(), &value);
            } break;
            case MetaType::VECTOR4: {
                Vector4 value = it.defaultValue.toVector4();
                instance->setVector4(it.name.c_str(), &value);
            } break;
            case MetaType::MATRIX4: {
                Matrix4 value = it.defaultValue.toMatrix4();
                instance->setMatrix4(it.name.c_str(), &value);
            } break;
            default: break;
            }
        }
    }
}
