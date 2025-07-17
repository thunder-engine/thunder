#include "resources/computeshadermt.h"

#include "commandbuffermt.h"

#include "resources/texturemt.h"
#include "resources/computebuffermt.h"

#include <cstring>

#include <log.h>

ComputeShaderMt::ComputeShaderMt() :
        m_computePipeline(nullptr) {

}

void ComputeShaderMt::loadUserData(const VariantMap &data) {
    ComputeShader::loadUserData(data);

    auto it = data.find("Shader");
    if(it != data.end()) {        
        m_shaderSource = (*it).second.toString();
    }

    switchState(ToBeUpdated);
}

MTL::ComputePipelineState *ComputeShaderMt::getProgramState() {
    switch(state()) {
        case Unloading: {
            if(m_computePipeline != nullptr) {
                m_computePipeline->release();
            }
            m_computePipeline = nullptr;

            switchState(ToBeDeleted);
        } break;
        case ToBeUpdated: {
            if(m_computePipeline != nullptr) {
                m_computePipeline->release();
            }

            NS::Error *error = nullptr;

            MTL::Library *library = WrapperMt::device()->newLibrary(NS::String::string(m_shaderSource.data(), NS::UTF8StringEncoding), nullptr, &error);
            if(library == nullptr) {
                aError() << error->localizedDescription()->utf8String();
                return nullptr;
            }

            MTL::Function *function = library->newFunction(NS::String::string("main", NS::UTF8StringEncoding));

            m_computePipeline = WrapperMt::device()->newComputePipelineState(function, &error);
            if(m_computePipeline == nullptr) {
                aError() << error->localizedDescription()->utf8String();
                return nullptr;
            }

            function->release();
            library->release();

            switchState(Ready);
        } break;
        default: break;
    }

    return m_computePipeline;
}

ComputeInstance *ComputeShaderMt::createInstance() {
    ComputeInstanceMt *result = new ComputeInstanceMt(this);

    initInstance(result);

    return result;
}

uint32_t ComputeShaderMt::uniformSize() const {
    return m_uniformSize;
}

ComputeInstanceMt::ComputeInstanceMt(ComputeShader *compute) :
        ComputeInstance(compute),
        m_data(nullptr) {

}

ComputeInstanceMt::~ComputeInstanceMt() {

}

bool ComputeInstanceMt::bind(CommandBufferMt *buffer, MTL::ComputeCommandEncoder *encoder) {
    ComputeShaderMt *shader = static_cast<ComputeShaderMt *>(m_compute);

    MTL::ComputePipelineState *state = shader->getProgramState();
    if(state) {
        encoder->setComputePipelineState(state);

        uint32_t size = shader->uniformSize();
        if(size) {
            if(m_data == nullptr) {
                m_data = WrapperMt::device()->newBuffer(m_uniformBuffer, size, MTL::ResourceStorageModeManaged);
            }

            if(m_uniformDirty) {
                m_data->didModifyRange(NS::Range::Make(0, size));
                m_uniformDirty = false;
            }

            encoder->setBuffer(m_data, 0, UNIFORM_BIND);
        }

        {
            uint8_t i = 0;
            for(auto &it : shader->textures()) {
                Texture *tex = it.texture;
                Texture *tmp = texture(it.name.data());

                if(tmp) {
                    tex = tmp;
                } else {
                    tmp = buffer->texture(it.name.data());
                    if(tmp) {
                        tex = tmp;
                    }
                }

                if(tex) {
                    TextureMt *textureMt = static_cast<TextureMt *>(tex);

                    encoder->setTexture(textureMt->nativeHandle(), i);
                }
                i++;
            }
        }
        {
            for(auto &it : shader->buffers()) {
                ComputeBuffer *computeBuffer = it.buffer;
                ComputeBuffer *tmp = ComputeInstance::buffer(it.name.data());

                if(tmp) {
                    computeBuffer = tmp;
                }

                if(computeBuffer) {
                    ComputeBufferMt *computeBufferMt = static_cast<ComputeBufferMt *>(computeBuffer);

                    encoder->setBuffer(computeBufferMt->nativeHandle(), 0, it.binding);
                }
            }
        }

        return true;
    }

    return false;
}

int32_t ComputeInstanceMt::maxTotalThreadsPerThreadgroup() const {
    ComputeShaderMt *shader = static_cast<ComputeShaderMt *>(m_compute);

    return shader->m_computePipeline->maxTotalThreadsPerThreadgroup();
}
