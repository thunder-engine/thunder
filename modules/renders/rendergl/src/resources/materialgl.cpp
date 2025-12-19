#include "resources/materialgl.h"

#include <cstring>

#include "agl.h"
#include "commandbuffergl.h"

#include "resources/texturegl.h"

#include <log.h>

const uint32_t gMaxUBO = 65536;

namespace  {
    const char *gInstanceData("InstanceData");
    const char *gGlobalData("Global");
};

void MaterialGL::loadUserData(const VariantMap &data) {
    Material::loadUserData(data);

    static std::map<std::string, uint32_t> pairs = {
        {"Visibility", FragmentVisibility},
        {"Default", FragmentDefault},

        {"Static", VertexStatic},
        {"Skinned", VertexSkinned},
        {"Particle", VertexParticle}
    };

    for(auto &pair : pairs) {
        auto it = data.find(pair.first);
        if(it != data.end()) {
            auto fields = (*it).second.toList();

            m_shaderSources[pair.second] = fields.front().toString(); // Shader data

            if(pair.second == FragmentVisibility) {
                m_layers |= Material::Visibility;
                if(m_layers & Opaque) {
                    m_layers |= Material::Shadowcast;
                }
            }
        }
    }

    switchState(ToBeUpdated);
}

uint32_t MaterialGL::getProgram(uint16_t type, int32_t &global, int32_t &local) {
    switch(state()) {
        case ToBeUpdated: {
            for(auto it : m_programs) {
                glDeleteProgram(it.second);
            }
            m_programs.clear();

            for(uint16_t v = Static; v < VertexLast; v++) {
                auto itv = m_shaderSources.find(v);
                if(itv != m_shaderSources.end()) {
                    for(uint16_t f = FragmentDefault; f < FragmentLast; f++) {
                        auto itf = m_shaderSources.find(f);
                        if(itf != m_shaderSources.end()) {
                            uint32_t vertex = buildShader(itv->first, itv->second);
                            uint32_t fragment = buildShader(itf->first, itf->second);

                            std::vector<uint32_t> shaders = {vertex, fragment};

                            uint32_t program = buildProgram(shaders, v);
                            m_programs[v * f] = program;
                            #ifdef THUNDER_MOBILE
                            m_globals[v * f] = glGetUniformBlockIndex(program, gGlobalData);
                            m_locals[v * f] = glGetUniformBlockIndex(program, gInstanceData);
                            #endif
                        }
                    }
                }
            }

            switchState(Ready);
        } break;
        case Unloading: {
            for(auto it : m_programs) {
                glDeleteProgram(it.second);
            }
            m_programs.clear();

            switchState(ToBeDeleted);
        } break;
        default: break;
    }

#ifdef THUNDER_MOBILE
    {
        auto it = m_globals.find(type);
        if(it != m_globals.end()) {
            global = it->second;
        }
    }
    {
        auto it = m_locals.find(type);
        if(it != m_locals.end()) {
            local = it->second;
        }
    }
#else
    global = GLOBAL_BIND;
    local = LOCAL_BIND;
#endif

    auto it = m_programs.find(type);
    if(it != m_programs.end()) {
        return it->second;
    }

    return 0;
}

uint32_t MaterialGL::buildShader(uint16_t type, const TString &src) {
    uint32_t t = 0;
    if(type >= FragmentDefault && type < FragmentLast) {
        t = GL_FRAGMENT_SHADER;
    } else if(type >= VertexStatic && type < VertexLast) {
        t = GL_VERTEX_SHADER;
    }

    uint32_t shader = glCreateShader(t);
    if(shader) {
        const char *data = src.data();

        glShaderSource(shader, 1, &data, nullptr);
        glCompileShader(shader);

        checkShader(shader);
    }

    return shader;
}

uint32_t MaterialGL::buildProgram(const std::vector<uint32_t> &shaders, uint16_t vertex) {
    uint32_t result = glCreateProgram();
    if(result) {
#ifndef THUNDER_MOBILE
        if(!name().isEmpty()) {
            CommandBufferGL::setObjectName(GL_PROGRAM, result, name());
        }
#endif
        for(auto it : shaders) {
            glAttachShader(result, it);
        }

        glLinkProgram(result);

        for(auto it : shaders) {
            glDetachShader(result, it);
            glDeleteShader(it);
        }

        checkProgram(result);

        glUseProgram(result);
        uint8_t t = 0;

        for(auto &it : m_textures) {
            int32_t location = glGetUniformLocation(result, it.name.data());
            if(location > -1) {
                glUniform1i(location, t);
            }
            t++;
        }
    }

    return result;
}

bool MaterialGL::checkShader(uint32_t shader) {
    int value = 0;

    glGetShaderiv(shader, GL_COMPILE_STATUS, &value);

    if(value != GL_TRUE) {
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &value);

        if(value) {
            std::string buff;
            buff.resize(value + 1);
            glGetShaderInfoLog(shader, value, nullptr, &buff[0]);

            aError() << "[ Render::MaterialGL ]" << name() << "\n[ Shader Said ]" << buff;
        }
        return false;
    }
    return true;
}

bool MaterialGL::checkProgram(uint32_t program) {
    int value = 0;

    glGetProgramiv(program, GL_LINK_STATUS, &value);

    if(value != GL_TRUE) {
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &value);
        if(value) {
            std::string buff;
            buff.resize(value + 1);
            glGetProgramInfoLog(program, value, nullptr, &buff[0]);

            aError() << "[ Render::MaterialGL ]" << name() << "\n[ Program Said ]" << buff;
        }
        return false;
    }
    return true;
}

MaterialInstance *MaterialGL::createInstance(SurfaceType type) {
    MaterialInstanceGL *result = new MaterialInstanceGL(this);

    if(state() == ToBeUpdated || state() == Ready) {
        initInstance(result);
    }

    result->setSurfaceType(type);

    return result;
}

void MaterialGL::switchState(State state) {
    Material::switchState(state);

    if(state == ToBeUpdated) {
        for(auto it : Material::m_instances) {
            initInstance(it);
        }
    }
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
        m_instanceBuffer(0),
        m_globalBuffer(0) {

    MaterialGL *m = static_cast<MaterialGL *>(material);
    m_glBlendState = m->m_blendState;

    // Blending
    m_glBlendState.colorOperation = convertBlendMode(m_glBlendState.colorOperation);
    m_glBlendState.alphaOperation = convertBlendMode(m_glBlendState.alphaOperation);

    m_glBlendState.sourceColorBlendMode = convertBlendFactor(m_glBlendState.sourceColorBlendMode);
    m_glBlendState.sourceAlphaBlendMode = convertBlendFactor(m_glBlendState.sourceAlphaBlendMode);

    m_glBlendState.destinationColorBlendMode = convertBlendFactor(m_glBlendState.destinationColorBlendMode);
    m_glBlendState.destinationAlphaBlendMode = convertBlendFactor(m_glBlendState.destinationAlphaBlendMode);

    // Depth
    m_glDepthState = m->m_depthState;
    m_glDepthState.compareFunction = 0x0200 | m_glDepthState.compareFunction;

    // Stencil
    m_glStencilState = m->m_stencilState;
    m_glStencilState.compareFunctionBack = 0x0200 | m_glStencilState.compareFunctionBack;
    m_glStencilState.compareFunctionFront = 0x0200 | m_glStencilState.compareFunctionFront;

    m_glStencilState.failOperationBack = convertAction(m_glStencilState.failOperationBack);
    m_glStencilState.failOperationFront = convertAction(m_glStencilState.failOperationFront);

    m_glStencilState.zFailOperationBack = convertAction(m_glStencilState.zFailOperationBack);
    m_glStencilState.zFailOperationFront = convertAction(m_glStencilState.zFailOperationFront);

    m_glStencilState.passOperationBack = convertAction(m_glStencilState.passOperationBack);
    m_glStencilState.passOperationFront = convertAction(m_glStencilState.passOperationFront);

}

MaterialInstanceGL::~MaterialInstanceGL() {
    if(m_instanceBuffer > 0) {
        glDeleteBuffers(1, &m_instanceBuffer);
        m_instanceBuffer = 0;
    }
}

uint32_t MaterialInstanceGL::drawsCount() const {
    const ByteArray &gpuBuffer = m_batchBuffer ? *m_batchBuffer : m_uniformBuffer;

    return (uint32_t)ceil((float)gpuBuffer.size() / (float)gMaxUBO);
}

bool MaterialInstanceGL::bind(CommandBufferGL *buffer, uint32_t layer, uint32_t index, const Global &global) {
    uint16_t type = MaterialGL::FragmentDefault;
    if((layer & Material::Visibility) || (layer & Material::Shadowcast)) {
        type = MaterialGL::FragmentVisibility;
    }

    MaterialGL *material = static_cast<MaterialGL *>(m_material);

    int32_t globalLocation = -1;
    int32_t instanceLocation = -1;
    uint32_t program = material->getProgram((m_surfaceType + 1) * type, globalLocation, instanceLocation);
    if(!program) {
        return false;
    }

    glUseProgram(program);

    uint32_t materialType = material->materialType();

    if(globalLocation > -1 && index == 0) {
        if(m_globalBuffer == 0) {
            glGenBuffers(1, &m_globalBuffer);

            int blockSize = -1;
            glGetActiveUniformBlockiv(program, globalLocation, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);

            glBindBuffer(GL_UNIFORM_BUFFER, m_globalBuffer);
            glBufferData(GL_UNIFORM_BUFFER, blockSize, nullptr, GL_DYNAMIC_DRAW);

            glBindBufferBase(GL_UNIFORM_BUFFER, globalLocation, m_globalBuffer);
            glUniformBlockBinding(program, globalLocation, globalLocation);
        }

        glBindBuffer(GL_UNIFORM_BUFFER, m_globalBuffer);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Global), &global);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        glBindBufferBase(GL_UNIFORM_BUFFER, globalLocation, m_globalBuffer);
    }

    uint32_t offset = index * gMaxUBO;

    ByteArray &gpuBuffer = m_batchBuffer ? *m_batchBuffer : rawUniformBuffer();
    int gpuBufferSize = MIN(gpuBuffer.size() - offset, gMaxUBO);

#ifdef THUNDER_MOBILE
    if(instanceLocation > -1) {
        if(m_instanceBuffer == 0) {
            glGenBuffers(1, &m_instanceBuffer);

            int blockSize = -1;
            glGetActiveUniformBlockiv(program, instanceLocation, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);

            glBindBuffer(GL_UNIFORM_BUFFER, m_instanceBuffer);
            glBufferData(GL_UNIFORM_BUFFER, blockSize, nullptr, GL_DYNAMIC_DRAW);

            glBindBufferBase(GL_UNIFORM_BUFFER, instanceLocation, m_instanceBuffer);
            glUniformBlockBinding(program, instanceLocation, instanceLocation);
        }

        glBindBuffer(GL_UNIFORM_BUFFER, m_instanceBuffer);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, gpuBufferSize, &gpuBuffer[offset]);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        glBindBufferBase(GL_UNIFORM_BUFFER, instanceLocation, m_instanceBuffer);
    }
#else
    if(m_instanceBuffer == 0) {
        glGenBuffers(1, &m_instanceBuffer);
    }

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_instanceBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, gpuBuffer.size(), gpuBuffer.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, instanceLocation, m_instanceBuffer);
#endif

    uint8_t i = 0;

    for(auto &it : material->textures()) {
        Texture *tex = texture(*buffer, it.binding);

        if(tex) {
            glActiveTexture(GL_TEXTURE0 + i);
            uint32_t texture = (tex->depth() > 1) ? GL_TEXTURE_3D : GL_TEXTURE_2D;
            if(tex->isCubemap()) {
                texture = GL_TEXTURE_CUBE_MAP;
            }

            glBindTexture(texture, static_cast<TextureGL *>(tex)->nativeHandle());
        }
        i++;
    }

    Material::RasterState rasterState;
    rasterState.cullingMode = GL_BACK;

    if(layer & Material::Shadowcast || materialType == Material::LightFunction) {
        rasterState.cullingMode = GL_FRONT;
    }

    if(material->doubleSided() || (layer & Material::Visibility)) {
        rasterState.enabled = false;
    }

    setRasterState(rasterState);

    Material::BlendState blendState = m_glBlendState;
    if(layer & Material::Visibility) {
        blendState.sourceColorBlendMode = GL_ONE;
        blendState.sourceAlphaBlendMode = GL_ONE;

        blendState.destinationColorBlendMode = GL_ZERO;
        blendState.destinationAlphaBlendMode = GL_ZERO;
    }

    setBlendState(blendState);

    setDepthState(m_glDepthState);

    setStencilState(m_glStencilState);

    return true;
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
