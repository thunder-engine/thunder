#include "resources/material.h"
#include "resources/texture.h"

#include "components/transform.h"

#include "commandbuffer.h"

#include <cstring>

namespace {
    const char *gProperties("Properties");
    const char *gTextures("Textures");
    const char *gUniforms("Uniforms");
    const char *gAttributes("Attributes");
    const char *gBlendState("BlendState");
    const char *gDepthState("DepthState");
    const char *gStencilState("StencilState");
}

/*!
    \class MaterialInstance
    \brief The MaterialInstance class represents an instance of a material, allowing for customization of material parameters.
    \inmodule Resources

    The MaterialInstance class enables customization of material parameters and textures for rendering objects.
    It supports various types of parameters, and the customization can be done per-instance.
*/

MaterialInstance::MaterialInstance(Material *material) :
        m_material(material),
        m_transform(nullptr),
        m_instanceCount(1),
        m_batchesCount(0),
        m_hash(material->uuid()),
        m_transformHash(0),
        m_surfaceType(0) {

}

MaterialInstance::~MaterialInstance() {

}

/*!
    Getter for the base material associated with the instance.
*/
Material *MaterialInstance::material() const {
    return m_material;
}
/*!
    Getter for the overridden texture associated with a specific parameter \a binding point.
*/
Texture *MaterialInstance::texture(CommandBuffer &buffer, int32_t binding) {
    auto it = m_textureOverride.find(binding);
    if(it != m_textureOverride.end()) {
        if(it->second) {
            return (*it).second;
        } else {
            for(auto t : m_material->m_textures) {
                if(t.binding == binding) {
                    Texture *texture = buffer.texture(t.name.c_str());
                    if(texture == nullptr) {
                        texture = Engine::loadResource<Texture>(".embedded/invalid.png");
                    }
                    //overrideTexture(binding, texture);
                    return texture;
                }
            }
        }
    }
    return nullptr;
}

void MaterialInstance::overrideTexture(int32_t binding, Texture *texture) {
    m_textureOverride[binding] = texture;
}
/*!
    Returns the number of GPU instances to be rendered.
*/
uint32_t MaterialInstance::instanceCount() const {
    return m_instanceCount + m_batchesCount;
}
/*!
    Sets the \a number of GPU instances to be rendered.
*/
void MaterialInstance::setInstanceCount(uint32_t number) {
    m_instanceCount = number;

    uint32_t istanceBufferSize = m_instanceCount * m_material->m_uniformSize;
    if(m_uniformBuffer.size() < istanceBufferSize) {
        m_uniformBuffer.resize(istanceBufferSize);
    }
}
/*!
    Returns a size of data per instance.
*/
uint32_t MaterialInstance::instanceSize() const {
    return m_material->m_uniformSize;
}
/*!
    Sets a boolean parameter with optional array support.
    Parameter \a name specifies a name of the boolean parameter.
    Parameter \a value pointer to the boolean value or array of boolean values.
    Parameter \a count a number of elements in the array.
*/
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
/*!
    Sets a integer parameter with optional array support.
    Parameter \a name specifies a name of the integer parameter.
    Parameter \a value pointer to the integer value or array of integer values.
    Parameter \a count a number of elements in the array.
*/
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
/*!
    Sets a float parameter with optional array support.
    Parameter \a name specifies a name of the float parameter.
    Parameter \a value pointer to the float value or array of float values.
    Parameter \a count a number of elements in the array.
*/
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
/*!
    Sets a Vector2 parameter with optional array support.
    Parameter \a name specifies a name of the Vector2 parameter.
    Parameter \a value pointer to the Vector2 value or array of Vector2 values.
    Parameter \a count a number of elements in the array.
*/
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
/*!
    Sets a Vector3 parameter with optional array support.
    Parameter \a name specifies a name of the Vector3 parameter.
    Parameter \a value pointer to the Vector3 value or array of Vector3 values.
    Parameter \a count a number of elements in the array.
*/
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
/*!
    Sets a Vector4 parameter with optional array support.
    Parameter \a name specifies a name of the Vector4 parameter.
    Parameter \a value pointer to the Vector4 value or array of Vector4 values.
    Parameter \a count a number of elements in the array.
*/
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
/*!
    Sets a Matrix4 parameter with optional array support.
    Parameter \a name specifies a name of the Matrix4 parameter.
    Parameter \a value pointer to the Matrix4 value or array of Matrix4 values.
    Parameter \a count a number of elements in the array.
*/
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
/*!
    Returns the a transform component.
*/
Transform *MaterialInstance::transform() {
    return m_transform;
}
/*!
    Sets the \a transform component to track it.
*/
void MaterialInstance::setTransform(Transform *transform) {
    m_transform = transform;
}
/*!
    Sets the \a transform matrix.
*/
void MaterialInstance::setTransform(const Matrix4 &transform) {
    memcpy(m_uniformBuffer.data(), &transform, sizeof(Matrix4));
}
/*!
    Sets the \a value of a parameter with specified \a name in the uniform buffer.
*/
void MaterialInstance::setBufferValue(const char *name, const void *value) {
    for(auto &it : m_material->m_uniforms) {
        if(it.name == name) {
            memcpy(&m_uniformBuffer[it.offset], value, it.size);

            break;
        }
    }
}
/*!
    Sets a \a texture parameter with specified \a name.
*/
void MaterialInstance::setTexture(const char *name, Texture *texture) {
    bool changed = false;
    for(auto it : m_material->m_textures) {
        if(it.name == name) {
            auto tIt = m_textureOverride.find(it.binding);
            if(tIt != m_textureOverride.end()) {
                Texture *old = (*tIt).second;
                if(old != texture) {
                    changed = true;
                    overrideTexture(it.binding, texture);
                }
            }
            break;
        }
    }

    if(changed) {
        m_hash = m_material->uuid();
        for(auto &it : m_textureOverride) {
            if(it.second) {
                Mathf::hashCombine(m_hash, it.second->uuid());
            }
        }
    }
}
/*!
    Gets the total count of parameters in the material.
*/
uint32_t MaterialInstance::paramCount() const {
    return m_material->m_uniforms.size();
}
/*!
    Gets the name of a parameter by \a index.
*/
std::string MaterialInstance::paramName(uint32_t index) const {
    if(index < m_material->m_uniforms.size()) {
        return m_material->m_uniforms[index].name;
    }
    return std::string();
}
/*!
    Gets the overridden or default value of a parameter by \a index.
*/
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
/*!
    Gets the surface type associated with the material instance.
*/
uint16_t MaterialInstance::surfaceType() const {
    return m_surfaceType;
}
/*!
    Sets the surface \a type associated with the material instance.
*/
void MaterialInstance::setSurfaceType(uint16_t type) {
    m_surfaceType = type;
}
/*!
    Returns a reference to CPU part of uniform buffer.
    Developer can modify it for their needs.
*/
ByteArray &MaterialInstance::rawUniformBuffer() {
    if(m_transform) {
        uint32_t hash = m_transform->hash();
        if(hash != m_transformHash) {
            Matrix4 m(m_transform->worldTransform());
            Vector4 color(CommandBuffer::idToColor(m_transform->actor()->uuid()));
            m[3] = color.x;
            m[7] = color.y;
            m[11] = color.z;
            m[15] = color.w;

            memcpy(m_uniformBuffer.data(), &m, sizeof(Matrix4));

            m_transformHash = static_cast<uint32_t>(hash);
        }
    }

    return m_uniformBuffer;
}

/*!
    Batches a material \a instance to draw using GPU instancing.
*/
void MaterialInstance::batch(MaterialInstance &instance) {
    if(m_batchBuffer.empty()) {
        ByteArray &buffer = rawUniformBuffer();
        m_batchBuffer.insert(m_batchBuffer.begin(), buffer.begin(), buffer.end());
    }

    ByteArray &buffer = instance.rawUniformBuffer();
    m_batchBuffer.insert(m_batchBuffer.end(), buffer.begin(), buffer.end());

    m_batchesCount += instance.m_instanceCount;
}

/*!
    Rests batch buffer.
*/
void MaterialInstance::resetBatches() {
    m_batchBuffer.clear();
    m_batchesCount = 0;
}
/*!
    \internal
*/
int MaterialInstance::hash() const {
    return static_cast<int32_t>(m_hash);
}
/*!
    \class Material
    \brief A Material is a resource which can be applied to a Mesh to control the visual look of the scene.
    \inmodule Resources
*/

Material::Material() :
        m_uniformSize(0),
        m_lightModel(Unlit),
        m_materialType(Surface),
        m_doubleSided(true),
        m_wireframe(false) {

}

Material::~Material() {

}
/*!
    Returns current material type.
    For more detalse please refer to Material::Type enum.
*/
int Material::materialType() const {
    return m_materialType;
}
/*!
    Sets new material \a type.
    For more detalse please refer to Material::Type enum.
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
        auto it = data.find(gProperties);
        if(it != data.end()) {
            VariantList list = (*it).second.value<VariantList>();
            auto i = list.begin();
            setMaterialType((*i).toInt());
            i++;
            setDoubleSided((*i).toBool());
            i++;
            setLightModel((*i).toInt());
            i++;
            setWireframe((*i).toBool());
        }

        it = data.find(gBlendState);
        if(it != data.end()) {
            loadBlendState((*it).second.value<VariantList>());
        }
        it = data.find(gDepthState);
        if(it != data.end()) {
            loadDepthState((*it).second.value<VariantList>());
        }
        it = data.find(gStencilState);
        if(it != data.end()) {
            loadStencilState((*it).second.value<VariantList>());
        }
    }
    {
        m_textures.clear();
        auto it = data.find(gTextures);
        if(it != data.end()) {
            for(auto &t : (*it).second.toList()) {
                VariantList list = t.toList();
                auto f = list.begin();
                std::string path = (*f).toString();
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
        size_t offset = sizeof(Matrix4);
        m_uniforms.clear();
        auto it = data.find(gUniforms);
        if(it != data.end()) {
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
        }
        m_uniformSize = offset;
    }
    {
        m_attributes.clear();
        auto it = data.find(gAttributes);
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
    \internal
*/
void Material::loadBlendState(const VariantList &data) {
    auto i = data.begin();
    m_blendState.alphaOperation = (*i).toInt();
    ++i;
    m_blendState.colorOperation = (*i).toInt();
    ++i;
    m_blendState.destinationAlphaBlendMode = (*i).toInt();
    ++i;
    m_blendState.destinationColorBlendMode = (*i).toInt();
    ++i;
    m_blendState.sourceAlphaBlendMode = (*i).toInt();
    ++i;
    m_blendState.sourceColorBlendMode = (*i).toInt();
    ++i;
    m_blendState.enabled = (*i).toBool();

}
/*!
    \internal
*/
void Material::loadDepthState(const VariantList &data) {
    auto i = data.begin();
    m_depthState.compareFunction = (*i).toInt();
    ++i;
    m_depthState.writeEnabled = (*i).toBool();
    ++i;
    m_depthState.enabled = (*i).toBool();

}

void Material::loadStencilState(const VariantList &data) {
    auto i = data.begin();
    m_stencilState.compareFunctionBack = (*i).toInt();
    ++i;
    m_stencilState.compareFunctionFront = (*i).toInt();
    ++i;
    m_stencilState.failOperationBack = (*i).toInt();
    ++i;
    m_stencilState.failOperationFront = (*i).toInt();
    ++i;
    m_stencilState.passOperationBack = (*i).toInt();
    ++i;
    m_stencilState.passOperationFront = (*i).toInt();
    ++i;
    m_stencilState.zFailOperationBack = (*i).toInt();
    ++i;
    m_stencilState.zFailOperationFront = (*i).toInt();
    ++i;
    m_stencilState.readMask = (*i).toInt();
    ++i;
    m_stencilState.writeMask = (*i).toInt();
    ++i;
    m_stencilState.reference = (*i).toInt();
    ++i;
    m_stencilState.enabled = (*i).toBool();

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
void Material::switchState(State state) {
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
        instance->m_uniformBuffer.resize(m_uniformSize);

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

        for(auto it : m_textures) {
            instance->overrideTexture(it.binding, it.texture);
        }

        instance->setTransform(Matrix4());
    }
}
