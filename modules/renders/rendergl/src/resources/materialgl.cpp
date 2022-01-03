#include "resources/materialgl.h"

#include <cstring>

#include "agl.h"
#include "commandbuffergl.h"

#include "resources/text.h"
#include "resources/texturegl.h"

#include <file.h>
#include <log.h>

MaterialGL::MaterialGL() :
    m_uniformSize(0) {

}

void MaterialGL::loadUserData(const VariantMap &data) {
    Material::loadUserData(data);

    if(m_MaterialType == Surface) {
        {
            auto it = data.find("Simple");
            if(it != data.end()) {
                m_ShaderSources[Simple] = (*it).second.toString();
            }
        }
        {
            auto it = data.find("StaticInst");
            if(it != data.end()) {
                m_ShaderSources[Instanced] = (*it).second.toString();
            }
        }
        {
            auto it = data.find("Particle");
            if(it != data.end()) {
                m_ShaderSources[Particle] = (*it).second.toString();
            }
        }
        {
            auto it = data.find("Skinned");
            if(it != data.end()) {
                m_ShaderSources[Skinned] = (*it).second.toString();
                setTexture("skinMatrices", nullptr);
            }
        }
    }

    {
        auto it = data.find("Shader");
        if(it != data.end()) {
            m_ShaderSources[Default] = (*it).second.toString();
        }
    }
    {
        auto it = data.find("Static");
        if(it != data.end()) {
            m_ShaderSources[Static] = (*it).second.toString();
        }
    }

    m_uniformSize = 0;
    for(auto &it : m_Uniforms) {
        m_uniformSize += it.size;
    }

    switchState(ToBeUpdated);
}

uint32_t MaterialGL::getProgram(uint16_t type) {
    switch(state()) {
        case Unloading: {
            for(auto it : m_Programs) {
                glDeleteProgram(it.second);
            }
            m_Programs.clear();

            switchState(ToBeDeleted);
        } break;
        case ToBeUpdated: {
            for(auto it : m_Programs) {
                glDeleteProgram(it.second);
            }
            m_Programs.clear();

            for(uint16_t v = Static; v < LastVertex; v++) {
                auto itv = m_ShaderSources.find(v);
                if(itv != m_ShaderSources.end()) {
                    for(uint16_t f = Default; f < LastFragment; f++) {
                        auto itf = m_ShaderSources.find(f);
                        if(itf != m_ShaderSources.end()) {
                            uint32_t vertex = buildShader(itv->first, itv->second);
                            uint32_t fragment = buildShader(itf->first, itf->second);
                            uint32_t index = v * f;
                            m_Programs[index] = buildProgram(vertex, fragment);
                        }
                    }
                }
            }

            switchState(Ready);
        } break;
        default: break;
    }

    auto it = m_Programs.find(type);
    if(it != m_Programs.end()) {
        return it->second;
    }
    return 0;
}

void MaterialGL::switchState(ResourceState state) {
    setState(state);
}

uint32_t MaterialGL::bind(uint32_t layer, uint16_t vertex) {
    int32_t b = blendMode();

    if((layer & CommandBuffer::DEFAULT || layer & CommandBuffer::SHADOWCAST) &&
       (b == Material::Additive || b == Material::Translucent)) {
        return 0;
    }
    if(layer & CommandBuffer::TRANSLUCENT && b == Material::Opaque) {
        return 0;
    }

    uint16_t type = MaterialGL::Default;
    if((layer & CommandBuffer::RAYCAST) || (layer & CommandBuffer::SHADOWCAST)) {
        type = MaterialGL::Simple;
    }
    uint32_t program = getProgram(vertex * type);
    if(!program) {
        return 0;
    }

    if(!m_DepthTest) {
        glDisable(GL_DEPTH_TEST);
    } else {
        glEnable(GL_DEPTH_TEST);
        //glDepthFunc((layer & CommandBuffer::DEFAULT) ? GL_EQUAL : GL_LEQUAL);

        glDepthMask((m_DepthWrite) ? GL_TRUE : GL_FALSE);
    }

    if(!doubleSided() && !(layer & CommandBuffer::RAYCAST)) {
        glEnable( GL_CULL_FACE );
        if(m_MaterialType == LightFunction) {
            glCullFace(GL_FRONT);
        } else {
            glCullFace(GL_BACK);
        }
    } else {
        glDisable(GL_CULL_FACE);
    }

    if(b != Material::Opaque && !(layer & CommandBuffer::RAYCAST)) {
        glEnable(GL_BLEND);
        if(b == Material::Translucent) {
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        } else {
            glBlendFunc(GL_ONE, GL_ONE);
        }
        glBlendEquation(GL_FUNC_ADD);
    } else {
        glDisable(GL_BLEND);
    }

    return program;
}

uint32_t MaterialGL::buildShader(uint16_t type, const string &src) {
    const char *data = src.c_str();

    uint32_t t;
    switch(type) {
        case Default:
        case Simple: {
            t  = GL_FRAGMENT_SHADER;
        } break;
        default: {
            t  = GL_VERTEX_SHADER;
        } break;
    }
    uint32_t shader = glCreateShader(t);
    if(shader) {
        glShaderSource(shader, 1, &data, nullptr);
        glCompileShader(shader);
        checkShader(shader, "");
    }

    return shader;
}

uint32_t MaterialGL::buildProgram(uint32_t vertex, uint32_t fragment) {
    uint32_t result = glCreateProgram();
    if(result) {
        glAttachShader(result, vertex);
        glAttachShader(result, fragment);
        glLinkProgram(result);
        glDetachShader(result, vertex);
        glDetachShader(result, fragment);

        checkShader(result, "", true);

        glDeleteShader(vertex);
        glDeleteShader(fragment);

        glUseProgram(result);
        uint8_t t = 0;
        for(auto &it : m_Textures) {
            int32_t location = glGetUniformLocation(result, it.name.c_str());
            if(location > -1) {
                glUniform1i(location, t);
            }
            t++;
        }
    }

    return result;
}

bool MaterialGL::checkShader(uint32_t shader, const string &path, bool link) {
    int value   = 0;

    if(!link) {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &value);
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &value);
    }

    if(value != GL_TRUE) {
        if(!link) {
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &value);
        } else {
            glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &value);
        }
        if(value) {
            char *buff  = new char[value + 1];
            if(!link) {
                glGetShaderInfoLog(shader, value, nullptr, buff);
            } else {
                glGetProgramInfoLog(shader, value, nullptr, buff);
            }
            Log(Log::ERR) << "[ Render::ShaderGL ]" << path.c_str() << "\n[ Said ]" << buff;
            delete []buff;
        }
        return false;
    }
    return true;
}

MaterialInstance *MaterialGL::createInstance(SurfaceType type) {
    MaterialInstanceGL *result = new MaterialInstanceGL(this);

    initInstance(result);

    if(result) {
        uint16_t t = Instanced;
        switch(type) {
            case SurfaceType::Static: t = Static; break;
            case SurfaceType::Skinned: t = Skinned; break;
            case SurfaceType::Billboard: t = Particle; break;
            default: break;
        }

        result->setSurfaceType(t);
    }
    return result;
}

uint32_t MaterialGL::uniformSize() const {
    return m_uniformSize;
}

MaterialInstanceGL::MaterialInstanceGL(Material *material) :
    MaterialInstance(material),
    m_instanceUbo(0),
    m_uniformBuffer(nullptr),
    m_uniformDirty(true) {

    MaterialGL *m = static_cast<MaterialGL *>(m_material);
    uint32_t size = m->uniformSize();
    if(size) {
        m_uniformBuffer = new uint8_t[size];
    }
}

MaterialInstanceGL::~MaterialInstanceGL() {
    m_instanceUbo = 0;
    delete []m_uniformBuffer;
}

bool MaterialInstanceGL::bind(CommandBufferGL *buffer, uint32_t layer) {
    MaterialGL *material = static_cast<MaterialGL *>(m_material);
    uint32_t program = material->bind(layer, surfaceType());
    if(program) {
        glUseProgram(program);

        // Push global uniform values to shader
        /// \todo Must be removed in future. After the Light structure will be removed from shader
        for(const auto &it : buffer->params()) {
            int32_t location = glGetUniformLocation(program, it.first.c_str());
            if(location > -1) {
                const Variant &data = it.second;
                switch(data.type()) {
                    case MetaType::VECTOR2: glUniform2fv      (location, 1, data.toVector2().v); break;
                    case MetaType::VECTOR3: glUniform3fv      (location, 1, data.toVector3().v); break;
                    case MetaType::VECTOR4: glUniform4fv      (location, 1, data.toVector4().v); break;
                    case MetaType::MATRIX4: glUniformMatrix4fv(location, 1, GL_FALSE, data.toMatrix4().mat); break;
                    default:                glUniform1f       (location, data.toFloat()); break;
                }
            }
        }

        uint32_t size = material->uniformSize();
        if(size) {
            if(m_instanceUbo == 0) {
                glGenBuffers(1, &m_instanceUbo);
                glBindBuffer(GL_UNIFORM_BUFFER, m_instanceUbo);
                glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
                glBindBuffer(GL_UNIFORM_BUFFER, 0);
            }

            if(m_uniformDirty) {
                glBindBuffer(GL_UNIFORM_BUFFER, m_instanceUbo);
                glBufferSubData(GL_UNIFORM_BUFFER, 0, size, m_uniformBuffer);
                glBindBuffer(GL_UNIFORM_BUFFER, 0);
                m_uniformDirty = false;
            }

            glBindBufferBase(GL_UNIFORM_BUFFER, UNIFORM_BIND, m_instanceUbo);
        }

        uint8_t i = 0;
        for(auto &it : material->textures()) {
            Texture *tex = it.texture;
            Texture *tmp = texture(it.name.c_str());

            if(tmp) {
                tex = tmp;
            } else {
                tmp = buffer->texture(it.name.c_str());
                if(tmp) {
                    tex = tmp;
                }
            }

            if(tex) {
                glActiveTexture(GL_TEXTURE0 + i);
                uint32_t texture = GL_TEXTURE_2D;
                if(tex->isCubemap()) {
                    texture = GL_TEXTURE_CUBE_MAP;
                }

                glBindTexture(texture, static_cast<TextureGL *>(tex)->nativeHandle());
            }
            i++;
        }

        return true;
    }

    return false;
}

void MaterialInstanceGL::setInteger(const char *name, const int32_t *value, int32_t count) {
    setValue(name, value);
}

void MaterialInstanceGL::setFloat(const char *name, const float *value, int32_t count) {
    setValue(name, value);
}

void MaterialInstanceGL::setVector2(const char *name, const Vector2 *value, int32_t count) {
    setValue(name, value);
}

void MaterialInstanceGL::setVector3(const char *name, const Vector3 *value, int32_t count) {
    setValue(name, value);
}

void MaterialInstanceGL::setVector4(const char *name, const Vector4 *value, int32_t count) {
    setValue(name, value);
}

void MaterialInstanceGL::setMatrix4(const char *name, const Matrix4 *value, int32_t count) {
    setValue(name, value);
}

void MaterialInstanceGL::setValue(const char *name, const void *value) {
    MaterialGL *material = static_cast<MaterialGL *>(m_material);
    for(auto &it : material->m_Uniforms) {
        if(it.name == name) {
            if(m_uniformBuffer) {
                memcpy(&m_uniformBuffer[it.offset], value, it.size);
                m_uniformDirty = true;
            }
            break;
        }
    }
}
