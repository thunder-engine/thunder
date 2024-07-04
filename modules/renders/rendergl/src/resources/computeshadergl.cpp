#include "resources/computeshadergl.h"

#include "agl.h"
#include "commandbuffergl.h"

#include "resources/texturegl.h"
#include "resources/computebuffergl.h"

#include <cstring>

#include <log.h>

void ComputeShaderGL::loadUserData(const VariantMap &data) {
    ComputeShader::loadUserData(data);

    auto it = data.find("Shader");
    if(it != data.end()) {
        m_shaderSource = (*it).second.toString();
    }

    switchState(ToBeUpdated);
}

uint32_t ComputeShaderGL::getProgram() {
    switch(state()) {
        case Unloading: {
            if(m_program > 0) {
                glDeleteProgram(m_program);
            }
            m_program = 0;

            switchState(ToBeDeleted);
        } break;
        case ToBeUpdated: {
            if(m_program > 0) {
                glDeleteProgram(m_program);
            }

            uint32_t shader = buildShader(m_shaderSource);
            m_program = buildProgram(shader);

            switchState(Ready);
        } break;
        default: break;
    }

    return m_program;
}

uint32_t ComputeShaderGL::buildShader(const std::string &src) {
    uint32_t shader = 0;
#ifndef THUNDER_MOBILE
    const char *data = src.c_str();
    shader = glCreateShader(GL_COMPUTE_SHADER);
    if(shader) {
        glShaderSource(shader, 1, &data, nullptr);
        glCompileShader(shader);

        checkShader(shader, "");
    }
#endif
    return shader;
}

uint32_t ComputeShaderGL::buildProgram(uint32_t shader) {
    uint32_t result = 0;
#ifndef THUNDER_MOBILE
    result = glCreateProgram();
    if(result) {
        if(!name().empty()) {
            CommandBufferGL::setObjectName(GL_PROGRAM, result, name());
        }

        glAttachShader(result, shader);
        glLinkProgram(result);
        glDetachShader(result, shader);

        checkShader(result, "", true);

        glDeleteShader(shader);

        glUseProgram(result);
        uint8_t t = 0;
        for(auto &it : m_textures) {
            int32_t location = glGetUniformLocation(result, it.name.c_str());
            if(location > -1) {
                glUniform1i(location, t);
            }
            t++;
        }
    }
#endif
    return result;
}

bool ComputeShaderGL::checkShader(uint32_t shader, const std::string &path, bool link) {
    int value = 0;

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
            aError() << "[ Render::ShaderGL ]" << path << "\n[ Said ]" << buff;
            delete []buff;
        }
        return false;
    }
    return true;
}

ComputeInstance *ComputeShaderGL::createInstance() {
    ComputeInstanceGL *result = new ComputeInstanceGL(this);

    initInstance(result);

    return result;
}

uint32_t ComputeShaderGL::uniformSize() const {
    return m_uniformSize;
}

ComputeInstanceGL::ComputeInstanceGL(ComputeShader *compute) :
        ComputeInstance(compute),
        m_instanceUbo(0) {

}

ComputeInstanceGL::~ComputeInstanceGL() {
    m_instanceUbo = 0;
}

bool ComputeInstanceGL::bind(CommandBufferGL *buffer) {
#ifndef THUNDER_MOBILE
    ComputeShaderGL *shader = static_cast<ComputeShaderGL *>(m_compute);
    uint32_t program = shader->getProgram();
    if(program) {
        glUseProgram(program);

        uint32_t size = shader->uniformSize();
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

        {
            uint8_t i = 0;
            for(auto &it : shader->textures()) {
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
        }
        {
            uint8_t i = 0;
            for(auto &it : shader->buffers()) {
                ComputeBuffer *buff = it.buffer;
                ComputeBuffer *tmp = ComputeInstance::buffer(it.name.c_str());

                if(tmp) {
                    buff = tmp;
                }

                if(buff) {
                    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, it.binding, static_cast<ComputeBufferGL *>(buff)->nativeHandle());
                }
                i++;
            }
        }

        return true;
    }
#endif
    return false;
}
