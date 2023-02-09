#include "resources/computeshader.h"

#include "resources/texture.h"
#include "resources/computebuffer.h"

#define TEXTURES    "Textures"
#define BUFFERS     "Buffers"
#define UNIFORMS    "Uniforms"

ComputeInstance::ComputeInstance(ComputeShader *compute) :
        m_compute(compute),
        m_uniformBuffer(nullptr),
        m_uniformDirty(true) {

    if(m_compute->m_uniformSize > 0) {
        m_uniformBuffer = new uint8_t[m_compute->m_uniformSize];
    }
}

ComputeInstance::~ComputeInstance() {
    delete []m_uniformBuffer;
}

ComputeShader *ComputeInstance::compute() const {
    return m_compute;
}

void ComputeInstance::setBool(const char *name, const bool *value, int32_t count) {
    A_UNUSED(count);
    setValue(name, value);
}

void ComputeInstance::setInteger(const char *name, const int32_t *value, int32_t count) {
    A_UNUSED(count);
    setValue(name, value);
}

void ComputeInstance::setFloat(const char *name, const float *value, int32_t count) {
    A_UNUSED(count);
    setValue(name, value);
}

void ComputeInstance::setVector2(const char *name, const Vector2 *value, int32_t count) {
    A_UNUSED(count);
    setValue(name, value);
}

void ComputeInstance::setVector3(const char *name, const Vector3 *value, int32_t count) {
    A_UNUSED(count);
    setValue(name, value);
}

void ComputeInstance::setVector4(const char *name, const Vector4 *value, int32_t count) {
    A_UNUSED(count);
    setValue(name, value);
}

void ComputeInstance::setMatrix4(const char *name, const Matrix4 *value, int32_t count) {
    A_UNUSED(count);
    setValue(name, value);
}

void ComputeInstance::setValue(const char *name, const void *value) {
    for(auto &it : m_compute->m_uniforms) {
        if(it.name == name) {
            if(m_uniformBuffer) {
                memcpy(&m_uniformBuffer[it.offset], value, it.size);
                m_uniformDirty = true;
            }
            break;
        }
    }
}

Texture *ComputeInstance::texture(const char *name) {
    auto it = m_textureOverride.find(name);
    if(it != m_textureOverride.end()) {
        return (*it).second;
    }
    return nullptr;
}

void ComputeInstance::setTexture(const char *name, Texture *value) {
    A_UNUSED(name);
    A_UNUSED(value);

    m_textureOverride[name] = value;
}

ComputeBuffer *ComputeInstance::buffer(const char *name) {
    auto it = m_bufferOverride.find(name);
    if(it != m_bufferOverride.end()) {
        return (*it).second;
    }
    return nullptr;
}

void ComputeInstance::setBuffer(const char *name, ComputeBuffer *value) {
    A_UNUSED(name);
    A_UNUSED(value);

    m_bufferOverride[name] = value;
}


ComputeShader::ComputeShader() :
    m_uniformSize(0) {

}

ComputeShader::~ComputeShader() {

}
/*!
    \internal
*/
void ComputeShader::loadUserData(const VariantMap &data) {
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
        m_buffers.clear();
        auto it = data.find(BUFFERS);
        if(it != data.end()) {
            for(auto &t : (*it).second.toList()) {
                VariantList list = t.toList();
                auto f = list.begin();
                string path = (*f).toString();
                BufferItem item;
                item.buffer = nullptr;
                if(!path.empty()) {
                    item.buffer = Engine::loadResource<ComputeBuffer>(path);
                }
                ++f;
                item.binding = (*f).toInt();
                ++f;
                item.name = (*f).toString();
                ++f;
                item.flags = (*f).toInt();

                m_buffers.push_back(item);
            }
        }
    }
    {
        m_uniformSize = 0;
        m_uniforms.clear();
        auto it = data.find(UNIFORMS);
        if(it != data.end()) {
            size_t offset = 0;
            for(auto &u : (*it).second.toList()) {
                VariantList list = u.toList();
                auto f = list.begin();

                UniformItem item;
                item.value = (*f); // value
                ++f;
                item.size = (*f).toInt(); // size
                ++f;
                item.name = (*f).toString(); // name

                item.offset = offset;
                offset += item.size;

                m_uniforms.push_back(item);
                m_uniformSize += item.size;
            }
        }
    }
}
/*!
    Returns a new instance for the compute shader.
*/
ComputeInstance *ComputeShader::createInstance() {
    ComputeInstance *result = new ComputeInstance(this);

    initInstance(result);

    return result;
}
/*!
    \internal
*/
void ComputeShader::switchState(ResourceState state) {
    setState(state);
}
/*!
    \internal
*/
bool ComputeShader::isUnloadable() {
    return true;
}
/*!
    \internal
*/
void ComputeShader::initInstance(ComputeInstance *instance) {
    if(instance) {
        for(auto &it : m_uniforms) {
            switch(it.value.type()) {
            case MetaType::INTEGER: {
                int32_t value = it.value.toInt();
                instance->setInteger(it.name.c_str(), &value);
            } break;
            case MetaType::FLOAT: {
                float value = it.value.toFloat();
                instance->setFloat(it.name.c_str(), &value);
            } break;
            case MetaType::VECTOR2: {
                Vector2 value = it.value.toVector2();
                instance->setVector2(it.name.c_str(), &value);
            } break;
            case MetaType::VECTOR3: {
                Vector3 value = it.value.toVector3();
                instance->setVector3(it.name.c_str(), &value);
            } break;
            case MetaType::VECTOR4: {
                Vector4 value = it.value.toVector4();
                instance->setVector4(it.name.c_str(), &value);
            } break;
            case MetaType::MATRIX4: {
                Matrix4 value = it.value.toMatrix4();
                instance->setMatrix4(it.name.c_str(), &value);
            } break;
            default: break;
            }
        }
    }
}
