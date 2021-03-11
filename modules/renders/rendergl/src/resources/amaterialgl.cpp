#include "resources/amaterialgl.h"

#include "agl.h"
#include "commandbuffergl.h"

#include "resources/text.h"
#include "resources/atexturegl.h"

#include <file.h>
#include <log.h>

void AMaterialGL::loadUserData(const VariantMap &data) {
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

    setState(ToBeUpdated);
}

uint32_t AMaterialGL::getProgram(uint16_t type) {
    switch(state()) {
        case Suspend: {
            for(auto it : m_Programs) {
                glDeleteProgram(it.second);
            }
            m_Programs.clear();

            setState(ToBeDeleted);
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

            setState(Ready);
        } break;
        default: break;
    }

    auto it = m_Programs.find(type);
    if(it != m_Programs.end()) {
        return it->second;
    }
    return 0;
}

uint32_t AMaterialGL::bind(uint32_t layer, uint16_t vertex) {
    int32_t b = blendMode();

    if((layer & ICommandBuffer::DEFAULT || layer & ICommandBuffer::SHADOWCAST) &&
       (b == Material::Additive || b == Material::Translucent)) {
        return 0;
    }
    if(layer & ICommandBuffer::TRANSLUCENT && b == Material::Opaque) {
        return 0;
    }

    uint16_t type = AMaterialGL::Default;
    if((layer & ICommandBuffer::RAYCAST) || (layer & ICommandBuffer::SHADOWCAST)) {
        type = AMaterialGL::Simple;
    }
    uint32_t program = getProgram(vertex * type);
    if(!program) {
        return 0;
    }

    if(!m_DepthTest) {
        glDisable(GL_DEPTH_TEST);
    } else {
        glEnable(GL_DEPTH_TEST);
        //glDepthFunc((layer & ICommandBuffer::DEFAULT) ? GL_EQUAL : GL_LEQUAL);

        glDepthMask((m_DepthWrite) ? GL_TRUE : GL_FALSE);
    }

    if(!doubleSided() && !(layer & ICommandBuffer::RAYCAST)) {
        glEnable( GL_CULL_FACE );
        if(m_MaterialType == LightFunction) {
            glCullFace(GL_FRONT);
        } else {
            glCullFace(GL_BACK);
        }
    } else {
        glDisable(GL_CULL_FACE);
    }

    if(b != Material::Opaque && !(layer & ICommandBuffer::RAYCAST)) {
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

uint32_t AMaterialGL::buildShader(uint16_t type, const string &src) {
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

uint32_t AMaterialGL::buildProgram(uint32_t vertex, uint32_t fragment) {
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
        for(auto it : m_Textures) {
            int32_t location = glGetUniformLocation(result, it.first.c_str());
            if(location > -1) {
                glUniform1i(location, t);
            }
            t++;
        }

        for(auto it : m_Uniforms) {
            int32_t location = glGetUniformLocation(result, it.first.c_str());
            if(location > -1) {
                switch(it.second.type()) {
                    case MetaType::VECTOR4: {
                        glUniform4fv(location, 1, it.second.toVector4().v);
                    } break;
                    default: {
                        glUniform1f(location, it.second.toFloat());
                    } break;
                }
            }
        }
    }

    return result;
}

bool AMaterialGL::checkShader(uint32_t shader, const string &path, bool link) {
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

MaterialInstance *AMaterialGL::createInstance(SurfaceType type) {
    MaterialInstance *result = Material::createInstance(type);
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
