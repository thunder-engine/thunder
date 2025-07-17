#include "resources/materialmt.h"

#include <cstring>

#include "commandbuffermt.h"

#include "components/transform.h"

#include "resources/texturemt.h"
#include "resources/rendertargetmt.h"

#include <log.h>

namespace {
const char *gGlobal("g");
const char *gLocal("instance");
const char *gUniform("uni");
}

enum ShaderType {
    VertexStatic      = 1,
    VertexSkinned,
    VertexParticle,
    VertexLast,

    FragmentDefault,
    FragmentVisibility,
    FragmentLast,
};

inline MTL::StencilOperation convertAction(int32_t action) {
    switch(action) {
    case Material::ActionType::Keep: return MTL::StencilOperationKeep;
    case Material::ActionType::Clear: return MTL::StencilOperationZero;
    case Material::ActionType::Replace: return MTL::StencilOperationReplace;
    case Material::ActionType::Increment: return MTL::StencilOperationIncrementClamp;
    case Material::ActionType::IncrementWrap: return MTL::StencilOperationIncrementWrap;
    case Material::ActionType::Decrement: return MTL::StencilOperationDecrementClamp;
    case Material::ActionType::DecrementWrap: return MTL::StencilOperationDecrementWrap;
    case Material::ActionType::Invert: return MTL::StencilOperationInvert;
    default: break;
    }

    return MTL::StencilOperationKeep;
}

inline MTL::BlendOperation convertBlendMode(int32_t mode) {
    switch(mode) {
    case Material::BlendOp::Add: return MTL::BlendOperationAdd;
    case Material::BlendOp::Subtract: return MTL::BlendOperationSubtract;
    case Material::BlendOp::ReverseSubtract: return MTL::BlendOperationReverseSubtract;
    case Material::BlendOp::Min: return MTL::BlendOperationMin;
    case Material::BlendOp::Max: return MTL::BlendOperationMax;
    default: break;
    }

    return MTL::BlendOperationAdd;
}

inline MTL::BlendFactor convertBlendFactor(int32_t factor) {
    switch(factor) {
    case Material::BlendFactor::Zero: return MTL::BlendFactorZero;
    case Material::BlendFactor::One: return MTL::BlendFactorOne;
    case Material::BlendFactor::SourceColor: return MTL::BlendFactorSourceColor;
    case Material::BlendFactor::OneMinusSourceColor: return MTL::BlendFactorOneMinusSourceColor;
    case Material::BlendFactor::DestinationColor: return MTL::BlendFactorDestinationColor;
    case Material::BlendFactor::OneMinusDestinationColor: return MTL::BlendFactorOneMinusDestinationColor;
    case Material::BlendFactor::SourceAlpha: return MTL::BlendFactorSourceAlpha;
    case Material::BlendFactor::OneMinusSourceAlpha: return MTL::BlendFactorOneMinusSourceAlpha;
    case Material::BlendFactor::DestinationAlpha: return MTL::BlendFactorDestinationAlpha;
    case Material::BlendFactor::OneMinusDestinationAlpha: return MTL::BlendFactorOneMinusDestinationAlpha;
    case Material::BlendFactor::SourceAlphaSaturate: return MTL::BlendFactorSourceAlphaSaturated;
    case Material::BlendFactor::ConstantColor: return MTL::BlendFactorBlendColor;
    case Material::BlendFactor::OneMinusConstantColor: return MTL::BlendFactorOneMinusBlendColor;
    case Material::BlendFactor::ConstantAlpha: return MTL::BlendFactorBlendAlpha;
    case Material::BlendFactor::OneMinusConstantAlpha: return MTL::BlendFactorOneMinusBlendAlpha;
    default: break;
    }

    return MTL::BlendFactorOne;
}

inline MTL::CompareFunction convertCompareFunction(int32_t function) {
    switch(function) {
    case Material::TestFunction::Never: return MTL::CompareFunctionNever;
    case Material::TestFunction::Less: return MTL::CompareFunctionLess;
    case Material::TestFunction::LessOrEqual: return MTL::CompareFunctionLessEqual;
    case Material::TestFunction::Greater:  return MTL::CompareFunctionGreater;
    case Material::TestFunction::GreaterOrEqual: return MTL::CompareFunctionGreaterEqual;
    case Material::TestFunction::Equal: return MTL::CompareFunctionEqual;
    case Material::TestFunction::NotEqual: return MTL::CompareFunctionNotEqual;
    case Material::TestFunction::Always: return MTL::CompareFunctionAlways;
    default: break;
    }

    return MTL::CompareFunction::CompareFunctionLess;
}

void MaterialMt::loadUserData(const VariantMap &data) {
    Material::loadUserData(data);

    if(m_depthStencilState) {
        m_depthStencilState->release();
    }

    MTL::DepthStencilDescriptor *desc = MTL::DepthStencilDescriptor::alloc()->init();
    if(m_depthState.enabled) {
        desc->setDepthCompareFunction( convertCompareFunction(m_depthState.compareFunction) );
        desc->setDepthWriteEnabled( m_depthState.writeEnabled );
    }

    if(m_stencilState.enabled) {
        MTL::StencilDescriptor *front = MTL::StencilDescriptor::alloc()->init();
        front->setStencilCompareFunction( convertCompareFunction(m_stencilState.compareFunctionFront) );
        front->setStencilFailureOperation( convertAction(m_stencilState.failOperationFront) );
        front->setDepthStencilPassOperation( convertAction(m_stencilState.passOperationFront) );
        front->setDepthFailureOperation( convertAction(m_stencilState.zFailOperationFront) );
        front->setReadMask(m_stencilState.readMask);
        front->setWriteMask(m_stencilState.writeMask);

        desc->setFrontFaceStencil(front);

        MTL::StencilDescriptor *back = MTL::StencilDescriptor::alloc()->init();
        back->setStencilCompareFunction( convertCompareFunction(m_stencilState.compareFunctionBack) );
        back->setStencilFailureOperation( convertAction(m_stencilState.failOperationBack) );
        back->setDepthStencilPassOperation( convertAction(m_stencilState.passOperationBack) );
        back->setDepthFailureOperation( convertAction(m_stencilState.zFailOperationBack) );
        back->setReadMask(m_stencilState.readMask);
        back->setWriteMask(m_stencilState.writeMask);

        desc->setBackFaceStencil(back);
    }

    m_depthStencilState = WrapperMt::device()->newDepthStencilState(desc);

    static const std::map<std::string, uint32_t> pairs = {
        {"Visibility", FragmentVisibility},
        {"Default", FragmentDefault},

        {"Static", VertexStatic},
        {"Skinned", VertexSkinned},
        {"Particle", VertexParticle},
    };

    for(auto it : m_pipelineFunctions) {
        it.second.function->release();
    }
    m_pipelineFunctions.clear();

    for(auto &pair : pairs) {
        auto it = data.find(pair.first);
        if(it != data.end()) {
            auto fields = (*it).second.toList();

            Shader shader;

            auto field = fields.begin(); // Shader data
            shader.function = buildShader(field->toString());
            ++field; // Uniform locations
            for(auto uniform : field->toList()) {
                VariantList list = uniform.toList();

                shader.uniforms.push_back({list.front().toString(), list.back().toInt()});
            }
            ++field; // Attributes
            if(field != fields.end()) {
                for(auto a : field->toList()) {
                    VariantList list = a.toList();

                    shader.attributes.push_back({list.back().toInt(), static_cast<uint32_t>(list.front().toInt())});
                }
            }

            m_pipelineFunctions[pair.second] = shader;
        }
    }
}

MTL::RenderPipelineState *MaterialMt::getPipeline(uint16_t vertex, uint16_t fragment, RenderTargetMt *target) {
    switch(state()) {
        case ToBeUpdated: {
            setState(Ready);
        } break;
        default: break;
    }

    uint32_t index = target->uuid();
    Mathf::hashCombine(index, vertex);
    Mathf::hashCombine(index, fragment);

    auto it = m_pipelines.find(index);
    if(it != m_pipelines.end()) {
        return it->second;
    } else {
        MTL::RenderPipelineState *pipeline = buildPipeline(vertex, fragment, target);
        if(pipeline) {
            m_pipelines[index] = pipeline;
            return pipeline;
        }
    }
    return nullptr;
}

bool MaterialMt::bind(MTL::RenderCommandEncoder *encoder, RenderTargetMt *target, uint32_t layer, uint16_t vertex) {
    if((layer & CommandBuffer::DEFAULT || layer & CommandBuffer::SHADOWCAST) && m_blendState.enabled) {
        return false;
    }
    if(layer & CommandBuffer::TRANSLUCENT && !m_blendState.enabled) {
        return false;
    }

    uint16_t type = FragmentDefault;
    if((layer & CommandBuffer::RAYCAST) || (layer & CommandBuffer::SHADOWCAST)) {
        type = FragmentVisibility;
    }

    MTL::RenderPipelineState *pipeline = getPipeline(vertex, type, target);
    if(pipeline) {
        encoder->setRenderPipelineState(pipeline);

        return true;
    }
    return false;
}

MTL::RenderPipelineState *MaterialMt::buildPipeline(uint32_t v, uint32_t f, RenderTargetMt *target) {
    MaterialMt::Shader *vertex = shader(v);
    MaterialMt::Shader *fragment = shader(f);

    MTL::VertexDescriptor *vertexDescriptor = MTL::VertexDescriptor::alloc()->init();

    const int uniformsCount = 2;
    for(int i = 0; i < vertex->attributes.size(); i++) {
        MTL::VertexAttributeDescriptor *attributeDesc = MTL::VertexAttributeDescriptor::alloc()->init();

        attributeDesc->setBufferIndex(i + uniformsCount);
        attributeDesc->setOffset(0);

        int size = 0;
        MTL::VertexFormat format = MTL::VertexFormatInvalid;
        switch (vertex->attributes[i].format) {
            case MetaType::VECTOR2: format = MTL::VertexFormatFloat2; size = sizeof(Vector2); break;
            case MetaType::VECTOR3: format = MTL::VertexFormatFloat3; size = sizeof(Vector3); break;
            case MetaType::VECTOR4: format = MTL::VertexFormatFloat4; size = sizeof(Vector4); break;
            default: break;
        }

        attributeDesc->setFormat(format);
        vertexDescriptor->attributes()->setObject(attributeDesc, i);

        MTL::VertexBufferLayoutDescriptor *layoutDesc = MTL::VertexBufferLayoutDescriptor::alloc()->init();
        layoutDesc->setStride(size);
        vertexDescriptor->layouts()->setObject(layoutDesc, i + uniformsCount);

        attributeDesc->release();
        layoutDesc->release();
    }

    MTL::RenderPipelineDescriptor *descriptor = MTL::RenderPipelineDescriptor::alloc()->init();
    descriptor->setVertexFunction(vertex->function);
    descriptor->setFragmentFunction(fragment->function);
    descriptor->setVertexDescriptor(vertexDescriptor);

    if(target->isNative()) {
        descriptor->colorAttachments()->object(0)->setPixelFormat(MTL::PixelFormatBGRA8Unorm);
    } else {
        for(int i = 0; i < target->colorAttachmentCount(); i++) {
            TextureMt *texture = static_cast<TextureMt *>(target->colorAttachment(i));

            MTL::RenderPipelineColorAttachmentDescriptor *attachDesc = MTL::RenderPipelineColorAttachmentDescriptor::alloc()->init();

            attachDesc->setPixelFormat(texture->pixelFormat());
            if(m_blendState.enabled) {
                attachDesc->setBlendingEnabled(true);

                attachDesc->setRgbBlendOperation(convertBlendMode(m_blendState.colorOperation));
                attachDesc->setAlphaBlendOperation(convertBlendMode(m_blendState.alphaOperation));

                attachDesc->setSourceRGBBlendFactor(convertBlendFactor(m_blendState.sourceColorBlendMode));
                attachDesc->setSourceAlphaBlendFactor(convertBlendFactor(m_blendState.sourceAlphaBlendMode));

                attachDesc->setDestinationRGBBlendFactor(convertBlendFactor(m_blendState.destinationColorBlendMode));
                attachDesc->setDestinationAlphaBlendFactor(convertBlendFactor(m_blendState.destinationAlphaBlendMode));
            }
            descriptor->colorAttachments()->setObject(attachDesc, i);

            attachDesc->release();
        }

        TextureMt *texture = static_cast<TextureMt *>(target->depthAttachment());
        if(texture) {
            descriptor->setDepthAttachmentPixelFormat(texture->pixelFormat());
        }
    }

    NS::Error *error;
    MTL::RenderPipelineState *result = WrapperMt::device()->newRenderPipelineState(descriptor, &error);
    if(result == nullptr) {
        aError()  << "[ Render::MaterialMT ]" << name() << error->localizedDescription()->utf8String();
    }

    vertexDescriptor->release();
    descriptor->release();

    return result;
}

MaterialMt::Shader *MaterialMt::shader(uint16_t type) {
    auto it = m_pipelineFunctions.find(type);
    if(it != m_pipelineFunctions.end()) {
        return &(it->second);
    }
    return nullptr;
}

MTL::Function *MaterialMt::buildShader(const TString &src) const {
    MTL::Function *result = nullptr;

    NS::Error *error = nullptr;
    MTL::Library *library = WrapperMt::device()->newLibrary(NS::String::string(src.data(), NS::UTF8StringEncoding), nullptr, &error);
    if(library != nullptr) {
        result = library->newFunction(NS::String::string("main0", NS::UTF8StringEncoding));
    } else {
        aError() << "[ Render::MaterialMT ]" << name() << "\n[ Shader Error ]\n" << error->localizedDescription()->utf8String();
    }

    return result;
}

MaterialInstance *MaterialMt::createInstance(SurfaceType type) {
    MaterialInstanceMt *result = new MaterialInstanceMt(this);

    initInstance(result);

    if(result) {
        result->setSurfaceType(type);
    }

    return result;
}

MaterialInstanceMt::MaterialInstanceMt(Material *material) :
        MaterialInstance(material),
        m_instanceBuffer(nullptr),
        m_globalVertextLocation(-1),
        m_localVertextLocation(-1),
        m_globalFragmentLocation(-1),
        m_localFragmentLocation(-1) {

}

MaterialInstanceMt::~MaterialInstanceMt() {
    if(m_instanceBuffer != nullptr) {
        m_instanceBuffer->release();
        m_instanceBuffer = 0;
    }
}

bool MaterialInstanceMt::bind(CommandBufferMt &buffer, uint32_t layer, const Global &global) {
    MTL::RenderCommandEncoder *encoder = buffer.encoder();

    MaterialMt *material = static_cast<MaterialMt *>(m_material);

    if(material->bind(encoder, buffer.currentRenderTarget(), layer, m_surfaceType + 1)) {
        if(m_globalVertextLocation == -1 && m_localVertextLocation == -1) {
            MaterialMt::Shader *shader = material->shader(VertexStatic);
            for(auto uniform : shader->uniforms) {
                if(uniform.name == gGlobal) {
                    m_globalVertextLocation = uniform.location;
                } else if(uniform.name == gLocal || uniform.name == gUniform) {
                    m_localVertextLocation = uniform.location;
                }
            }
        }

        if(m_globalFragmentLocation == -1 && m_localFragmentLocation == -1) {
            MaterialMt::Shader *shader = material->shader(FragmentDefault);
            for(auto uniform : shader->uniforms) {
                if(uniform.name == gGlobal) {
                    m_globalFragmentLocation = uniform.location;
                } else if(uniform.name == gLocal || uniform.name == gUniform) {
                    m_localFragmentLocation = uniform.location;
                }
            }
        }

        // Global buffer
        {
            if(m_globalBuffer == nullptr) {
                m_globalBuffer = WrapperMt::device()->newBuffer(sizeof(Global), MTL::ResourceStorageModeManaged);
            }
            uint8_t *ptr = reinterpret_cast<uint8_t *>(m_globalBuffer->contents());
            if(ptr) {
                memcpy(ptr, &global, sizeof(Global));
                m_globalBuffer->didModifyRange(NS::Range::Make(0, sizeof(Global)));
            }

            if(m_globalVertextLocation >= 0) {
                encoder->setVertexBuffer(m_globalBuffer, 0, m_globalVertextLocation);
            }
            if(m_globalFragmentLocation >= 0) {
                encoder->setFragmentBuffer(m_globalBuffer, 0, m_globalFragmentLocation);
            }
        }

        // Instance buffer
        {
            ByteArray &localBuffer = m_batchBuffer.empty() ? rawUniformBuffer() : m_batchBuffer;

            if(m_instanceBuffer == nullptr || m_instanceBuffer->length() < localBuffer.size()) {
                if(m_instanceBuffer) {
                    m_instanceBuffer->release();
                }
                m_instanceBuffer = WrapperMt::device()->newBuffer(localBuffer.size(), MTL::ResourceStorageModeManaged);
            }
            uint8_t *ptr = reinterpret_cast<uint8_t *>(m_instanceBuffer->contents());
            if(ptr) {
                memcpy(ptr, localBuffer.data(), localBuffer.size());
                m_instanceBuffer->didModifyRange(NS::Range::Make(0, localBuffer.size()));
            }

            if(m_localVertextLocation >= 0) {
                encoder->setVertexBuffer(m_instanceBuffer, 0, m_localVertextLocation);
            }
            if(m_localFragmentLocation >= 0) {
                encoder->setFragmentBuffer(m_instanceBuffer, 0, m_localFragmentLocation);
            }
        }

        for(auto &it : material->textures()) {
            TextureMt *tex = static_cast<TextureMt *>(texture(buffer, it.binding));
            if(tex && it.binding >= 0) {
                encoder->setFragmentTexture(tex->nativeHandle(), it.binding);
                encoder->setFragmentSamplerState(tex->sampler(), it.binding);
            }
        }

        MTL::DepthStencilState *state = material->depthStencilState();
        if(state) {
            encoder->setDepthStencilState(state);
            if(material->m_stencilState.enabled) {
                encoder->setStencilReferenceValue(material->m_stencilState.reference);
            }
        }

        return true;
    }

    return false;
}
