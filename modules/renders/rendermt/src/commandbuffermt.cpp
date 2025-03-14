#include "commandbuffermt.h"

#include <cstring>

#include "resources/meshmt.h"
#include "resources/materialmt.h"
#include "resources/rendertargetmt.h"
#include "resources/computeshadermt.h"

#include <timer.h>

CommandBufferMt::CommandBufferMt() :
        m_commandBuffer(nullptr),
        m_encoder(nullptr) {

}

void CommandBufferMt::begin(MTL::CommandBuffer *cmd) {
    PROFILE_FUNCTION();

    m_commandBuffer = cmd;

    m_global.time = Timer::time();
    m_global.deltaTime = Timer::deltaTime();
    m_global.clip = 0.1f;
}

void CommandBufferMt::end() {
    if(m_encoder) {
        m_encoder->endEncoding();
        m_encoder = nullptr;
    }
}

MTL::RenderCommandEncoder *CommandBufferMt::encoder() const {
    return m_encoder;
}

RenderTargetMt *CommandBufferMt::currentRenderTarget() const {
    return m_currentTarget;
}

void CommandBufferMt::dispatchCompute(ComputeInstance *shader, int32_t groupsX, int32_t groupsY, int32_t groupsZ) {
    PROFILE_FUNCTION();

    if(shader) {
        ComputeInstanceMt *instance = static_cast<ComputeInstanceMt *>(shader);

        MTL::ComputeCommandEncoder *computeEncoder = m_commandBuffer->computeCommandEncoder();
        if(instance->bind(this, computeEncoder)) {
            MTL::Size threadgroupSize(instance->maxTotalThreadsPerThreadgroup(), 1, 1);

            computeEncoder->dispatchThreads(MTL::Size(groupsX, groupsY, groupsZ), threadgroupSize);

            computeEncoder->endEncoding();
        }
    }
}

void CommandBufferMt::drawMesh(Mesh *mesh, uint32_t sub, uint32_t layer, MaterialInstance &instance) {
    PROFILE_FUNCTION();

    if(mesh && m_encoder) {
        MeshMt *meshMt = static_cast<MeshMt *>(mesh);

        MaterialInstanceMt &instanceMt = static_cast<MaterialInstanceMt &>(instance);
        if(instanceMt.bind(*this, layer, m_global)) {
            meshMt->bind(m_encoder, 2);

            bool wire = instance.material()->wireframe();

            m_encoder->setTriangleFillMode(wire ? MTL::TriangleFillModeLines : MTL::TriangleFillModeFill);

            if(meshMt->indices().empty()) {
                uint32_t vert = meshMt->vertices().size();

                MTL::PrimitiveType primitiveType = wire ? MTL::PrimitiveTypeLineStrip : MTL::PrimitiveTypeTriangleStrip;
                m_encoder->drawPrimitives(primitiveType, 0, vert, instance.instanceCount(), 0);

                PROFILER_STAT(POLYGONS, index - 2 * count);
            } else {
                int32_t index = meshMt->indexCount(sub);

                MTL::PrimitiveType primitiveType = wire ? MTL::PrimitiveTypeLine : MTL::PrimitiveTypeTriangle;
                m_encoder->drawIndexedPrimitives(primitiveType, index, MTL::IndexTypeUInt32, meshMt->indexBuffer(),
                                                 meshMt->indexStart(sub), instance.instanceCount(), 0, 0);

                PROFILER_STAT(POLYGONS, (index / 3) * count);
            }

            PROFILER_STAT(DRAWCALLS, 1);
        }
    }
}

void CommandBufferMt::setRenderTarget(RenderTarget *target, uint32_t level) {
    PROFILE_FUNCTION();

    m_currentTarget = static_cast<RenderTargetMt *>(target);

    if(m_currentTarget) {
        m_currentTarget->setLevel(level);

        if(m_encoder) {
            m_encoder->endEncoding();
            m_encoder = nullptr;
        }
        MTL::RenderPassDescriptor *descriptor = m_currentTarget->nativeHandle();
        if(descriptor) {
            m_encoder = m_commandBuffer->renderCommandEncoder(descriptor);
            m_encoder->setLabel(NS::String::string(target->name().c_str(), NS::UTF8StringEncoding));
        }
    }
}

void CommandBufferMt::setViewport(int32_t x, int32_t y, int32_t width, int32_t height) {
    CommandBuffer::setViewport(x, y, width, height);

    if(m_encoder) {
        m_encoder->setViewport({(float)m_viewportX, (float)m_viewportY, (float)m_viewportWidth, (float)m_viewportHeight, 0.0f, 1.0f});
    }
}

void CommandBufferMt::enableScissor(int32_t x, int32_t y, int32_t width, int32_t height) {
    if(m_encoder) {
        m_encoder->setScissorRect({(uint32_t)x, (uint32_t)y, (uint32_t)width, (uint32_t)height});
    }
}

void CommandBufferMt::disableScissor() {
    if(m_encoder) {
        m_encoder->setScissorRect({(uint32_t)m_viewportX, (uint32_t)m_viewportY, (uint32_t)m_viewportWidth, (uint32_t)m_viewportHeight});
    }
}
