#include "resources/materialgl.h"

#include <cstring>

#include "agl.h"
#include "commandbuffergl.h"

#include "resources/texturegl.h"

#include <log.h>

namespace  {
    const char *gVisibility("Visibility");
    const char *gDefault("Default");

    const char *gStatic("Static");
    const char *gStaticInst("StaticInst");
    const char *gSkinned("Skinned");
    const char *gParticle("Particle");
    const char *gFullscreen("Fullscreen");
};

void MaterialGL::loadUserData(const VariantMap &data) {
    Material::loadUserData(data);

    static map<string, uint32_t> pairs = {
        {gVisibility, Visibility},
        {gDefault, Default},

        {gStatic, Static},
        {gStaticInst, StaticInst},
        {gSkinned, Skinned},
        {gParticle, Particle},
        {gFullscreen, Fullscreen}
    };

    for(auto &pair : pairs) {
        auto it = data.find(pair.first);
        if(it != data.end()) {
            m_shaderSources[pair.second] = (*it).second.toString();
        }
    }

    switchState(ToBeUpdated);
}

uint32_t MaterialGL::getProgram(uint16_t type) {
    switch(state()) {
        case Unloading: {
            for(auto it : m_programs) {
                glDeleteProgram(it.second);
            }
            m_programs.clear();

            switchState(ToBeDeleted);
        } break;
        case ToBeUpdated: {
            for(auto it : m_programs) {
                glDeleteProgram(it.second);
            }
            m_programs.clear();

            for(uint16_t v = Static; v < LastVertex; v++) {
                auto itv = m_shaderSources.find(v);
                if(itv != m_shaderSources.end()) {
                    for(uint16_t f = Default; f < LastFragment; f++) {
                        auto itf = m_shaderSources.find(f);
                        if(itf != m_shaderSources.end()) {
                            uint32_t vertex = buildShader(itv->first, itv->second);
                            uint32_t fragment = buildShader(itf->first, itf->second);
                            uint32_t index = v * f;
                            m_programs[index] = buildProgram(vertex, fragment);
                        }
                    }
                }
            }

            switchState(Ready);
        } break;
        default: break;
    }

    auto it = m_programs.find(type);
    if(it != m_programs.end()) {
        return it->second;
    }
    return 0;
}

uint32_t MaterialGL::bind(uint32_t layer, uint16_t vertex) {
    if((layer & CommandBuffer::DEFAULT || layer & CommandBuffer::SHADOWCAST) && m_blendState.enabled) {
        return 0;
    }
    if(layer & CommandBuffer::TRANSLUCENT && !m_blendState.enabled) {
        return 0;
    }

    uint16_t type = MaterialGL::Default;
    if((layer & CommandBuffer::RAYCAST) || (layer & CommandBuffer::SHADOWCAST)) {
        type = Visibility;
    }

    uint32_t program = getProgram(vertex * type);
    if(!program) {
        return 0;
    }

    return program;
}

uint32_t MaterialGL::buildShader(uint16_t type, const string &src) {
    const char *data = src.c_str();

    uint32_t t;
    switch(type) {
        case Default:
        case Visibility: {
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
#ifndef THUNDER_MOBILE
        if(!name().empty()) {
            CommandBufferGL::setObjectName(GL_PROGRAM, result, name());
        }
#endif

        glAttachShader(result, vertex);
        glAttachShader(result, fragment);
        glLinkProgram(result);
        glDetachShader(result, vertex);
        glDetachShader(result, fragment);

        checkShader(result, true);

        glDeleteShader(vertex);
        glDeleteShader(fragment);

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

    return result;
}

bool MaterialGL::checkShader(uint32_t shader, bool link) {
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
            aError() << "[ Render::ShaderGL ]" << name().c_str() << "\n[ Said ]" << buff;
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
        uint16_t t = StaticInst;
        switch(type) {
            case SurfaceType::Static: t = ShaderType::Static; break;
            case SurfaceType::Skinned: t = ShaderType::Skinned; break;
            case SurfaceType::Billboard: t = ShaderType::Particle; break;
            case SurfaceType::Fullscreen: t = ShaderType::Fullscreen; break;
            default: break;
        }

        result->setSurfaceType(t);
    }

    return result;
}

uint32_t MaterialGL::uniformSize() const {
    return m_uniformSize;
}

inline int32_t convertAction(int32_t action) {
    switch(action) {
        case Material::ActionType::Keep: return GL_KEEP;
        case Material::ActionType::Clear: return GL_ZERO;
        case Material::ActionType::Replace: return GL_REPLACE;
        case Material::ActionType::Increment: return GL_INCR;
        case Material::ActionType::IncrementWrap: return GL_INCR_WRAP;
        case Material::ActionType::Decrement: return GL_DECR;
        case Material::ActionType::DecrementWrap: return GL_DECR_WRAP;
        case Material::ActionType::Invert: return GL_INVERT;
        default: break;
    }

    return action;
}

inline int32_t convertBlendMode(int32_t mode) {
    switch(mode) {
        case Material::BlendOp::Add: return GL_FUNC_ADD;
        case Material::BlendOp::Subtract: return GL_FUNC_SUBTRACT;
        case Material::BlendOp::ReverseSubtract: return GL_FUNC_REVERSE_SUBTRACT;
        case Material::BlendOp::Min: return GL_MIN;
        case Material::BlendOp::Max: return GL_MAX;
        default: break;
    }

    return mode;
}

inline int32_t convertBlendFactor(int32_t factor) {
    switch(factor) {
        case Material::BlendFactor::Zero: return GL_ZERO;
        case Material::BlendFactor::One: return GL_ONE;
        case Material::BlendFactor::SourceColor: return GL_SRC_COLOR;
        case Material::BlendFactor::OneMinusSourceColor: return GL_ONE_MINUS_SRC_COLOR;
        case Material::BlendFactor::DestinationColor: return GL_DST_COLOR;
        case Material::BlendFactor::OneMinusDestinationColor: return GL_ONE_MINUS_DST_COLOR;
        case Material::BlendFactor::SourceAlpha: return GL_SRC_ALPHA;
        case Material::BlendFactor::OneMinusSourceAlpha: return GL_ONE_MINUS_SRC_ALPHA;
        case Material::BlendFactor::DestinationAlpha: return GL_DST_ALPHA;
        case Material::BlendFactor::OneMinusDestinationAlpha: return GL_ONE_MINUS_DST_ALPHA;
        case Material::BlendFactor::SourceAlphaSaturate: return GL_SRC_ALPHA_SATURATE;
        case Material::BlendFactor::ConstantColor: return GL_CONSTANT_COLOR;
        case Material::BlendFactor::OneMinusConstantColor: return GL_ONE_MINUS_CONSTANT_COLOR;
        case Material::BlendFactor::ConstantAlpha: return GL_CONSTANT_ALPHA;
        case Material::BlendFactor::OneMinusConstantAlpha: return GL_ONE_MINUS_CONSTANT_ALPHA;
        default: break;
    }

    return factor;
}

MaterialInstanceGL::MaterialInstanceGL(Material *material) :
        MaterialInstance(material),
        m_instanceUbo(0) {

    MaterialGL *m = static_cast<MaterialGL *>(material);
    m_blendState = m->m_blendState;

    // Blending
    m_blendState.colorOperation = convertBlendMode(m_blendState.colorOperation);
    m_blendState.alphaOperation = convertBlendMode(m_blendState.alphaOperation);

    m_blendState.sourceColorBlendMode = convertBlendFactor(m_blendState.sourceColorBlendMode);
    m_blendState.sourceAlphaBlendMode = convertBlendFactor(m_blendState.sourceAlphaBlendMode);

    m_blendState.destinationColorBlendMode = convertBlendFactor(m_blendState.destinationColorBlendMode);
    m_blendState.destinationAlphaBlendMode = convertBlendFactor(m_blendState.destinationAlphaBlendMode);

    // Depth
    m_depthState = m->m_depthState;
    m_depthState.compareFunction = 0x0200 | m_depthState.compareFunction;

    // Stencil
    m_stencilState = m->m_stencilState;
    m_stencilState.compareFunctionBack = 0x0200 | m_stencilState.compareFunctionBack;
    m_stencilState.compareFunctionFront = 0x0200 | m_stencilState.compareFunctionFront;

    m_stencilState.failOperationBack = convertAction(m_stencilState.failOperationBack);
    m_stencilState.failOperationFront = convertAction(m_stencilState.failOperationFront);

    m_stencilState.zFailOperationBack = convertAction(m_stencilState.zFailOperationBack);
    m_stencilState.zFailOperationFront = convertAction(m_stencilState.zFailOperationFront);

    m_stencilState.passOperationBack = convertAction(m_stencilState.passOperationBack);
    m_stencilState.passOperationFront = convertAction(m_stencilState.passOperationFront);
}

MaterialInstanceGL::~MaterialInstanceGL() {
    if(m_instanceUbo > 0) {
        glDeleteBuffers(1, &m_instanceUbo);
        m_instanceUbo = 0;
    }
}

bool MaterialInstanceGL::bind(CommandBufferGL *buffer, uint32_t layer) {
    MaterialGL *material = static_cast<MaterialGL *>(m_material);
    uint32_t program = material->bind(layer, surfaceType());
    if(program) {
        glUseProgram(program);

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

        Material::RasterState rasterState;
        rasterState.cullingMode = GL_BACK;

        if(layer & CommandBuffer::SHADOWCAST) {
            rasterState.cullingMode = GL_FRONT;
        } else if(!material->doubleSided() && !(layer & CommandBuffer::RAYCAST)) {
            if(material->materialType() != Material::LightFunction) {
                rasterState.cullingMode = GL_BACK;
            }
        } else {
            rasterState.enabled = false;
        }

        setRasterState(rasterState);

        Material::BlendState blendState = m_blendState;
        if(layer & CommandBuffer::RAYCAST) {
            blendState.sourceColorBlendMode = GL_ONE;
            blendState.sourceAlphaBlendMode = GL_ONE;

            blendState.destinationColorBlendMode = GL_ZERO;
            blendState.destinationAlphaBlendMode = GL_ZERO;
        }

        setBlendState(blendState);

        setDepthState(m_depthState);

        setStencilState(m_stencilState);

        return true;
    }

    return false;
}

void MaterialInstanceGL::setBlendState(const Material::BlendState &state) {
    if(state.enabled) {
        glEnable(GL_BLEND);

        glBlendFuncSeparate(state.sourceColorBlendMode, state.destinationColorBlendMode, state.sourceAlphaBlendMode, state.destinationAlphaBlendMode);

        glBlendEquationSeparate(state.colorOperation, state.alphaOperation);
    } else {
        glDisable(GL_BLEND);
    }
}

void MaterialInstanceGL::setRasterState(const Material::RasterState &state) {
    if(state.enabled) {
        glEnable(GL_CULL_FACE);

        glCullFace(state.cullingMode);
    } else {
        glDisable(GL_CULL_FACE);
    }
}

void MaterialInstanceGL::setDepthState(const Material::DepthState &state) {
    if(state.enabled) {
        glEnable(GL_DEPTH_TEST);

        glDepthMask(state.writeEnabled);

        glDepthFunc(state.compareFunction);
    } else {
        glDisable(GL_DEPTH_TEST);
    }
}

void MaterialInstanceGL::setStencilState(const Material::StencilState &state) {
    if(state.enabled) {
        glEnable(GL_STENCIL_TEST);

        glStencilMask(state.writeMask);

        glStencilFuncSeparate(GL_BACK, state.compareFunctionBack, state.reference, state.readMask);
        glStencilFuncSeparate(GL_FRONT, state.compareFunctionFront, state.reference, state.readMask);

        glStencilOpSeparate(GL_BACK, state.failOperationBack, state.zFailOperationBack, state.passOperationBack);
        glStencilOpSeparate(GL_FRONT, state.failOperationFront, state.zFailOperationFront, state.passOperationFront);
    } else {
        glDisable(GL_STENCIL_TEST);
    }
}
