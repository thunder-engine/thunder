#include "commandbuffermt.h"

#include "resources/meshmt.h"
#include "resources/materialmt.h"
#include "resources/rendertargetmt.h"
#include "resources/computeshadermt.h"

CommandBufferMt::CommandBufferMt() :
        m_commandBuffer(nullptr),
        m_encoder(nullptr),
        m_currentFrame(0) {

}

void CommandBufferMt::begin(MTL::CommandBuffer *cmd) {
    PROFILE_FUNCTION();

    m_commandBuffer = cmd;
    m_currentFrame = (m_currentFrame + 1) % WrapperMt::framesInFlight();

    CommandBuffer::begin();
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

void CommandBufferMt::dispatchCompute(ComputeInstance &shader, int32_t groupsX, int32_t groupsY, int32_t groupsZ) {
    PROFILE_FUNCTION();

    ComputeInstanceMt &instance = static_cast<ComputeInstanceMt &>(shader);

    MTL::ComputeCommandEncoder *encoder = m_commandBuffer->computeCommandEncoder();
    if(instance.bind(this, encoder)) {
        MTL::Size threadgroupSize(instance.maxTotalThreadsPerThreadgroup(), 1, 1);

        encoder->dispatchThreads(MTL::Size(groupsX, groupsY, groupsZ), threadgroupSize);

        encoder->endEncoding();
    }
}

void CommandBufferMt::drawMesh(Mesh *mesh, uint32_t sub, uint32_t layer, MaterialInstance &instance) {
    PROFILE_FUNCTION();

    if(mesh && m_encoder) {
        MaterialInstanceMt &instanceMt = static_cast<MaterialInstanceMt &>(instance);
        if(instanceMt.bind(*this, layer, static_cast<RenderTargetMt *>(m_target)->globalBuffer(m_currentFrame), m_currentFrame)) {
            MeshMt *meshMt = static_cast<MeshMt *>(mesh);
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

    if(m_encoder) {
        m_encoder->endEncoding();
        m_encoder = nullptr;
    }

    CommandBuffer::setRenderTarget(target, level);

    RenderTargetMt *targetMt = static_cast<RenderTargetMt *>(m_target);
    if(targetMt) {
        targetMt->setLevel(level);

        if(!(targetMt->flags() & RenderTarget::Atlas)) {
            targetMt->updateGlobalMemory(m_currentFrame, m_global);
        }

        MTL::RenderPassDescriptor *descriptor = targetMt->nativeHandle();
        if(descriptor) {
            m_encoder = m_commandBuffer->renderCommandEncoder(descriptor);
            m_encoder->setLabel(NS::String::string(target->name().data(), NS::UTF8StringEncoding));
        }
    }
}

void CommandBufferMt::setViewport(int32_t x, int32_t y, int32_t width, int32_t height) {
    CommandBuffer::setViewport(x, y, width, height);

    if(m_target && m_target->flags() & RenderTarget::Atlas && m_target->tileIndex() >= 0) {
        static_cast<RenderTargetMt *>(m_target)->updateGlobalMemory(m_currentFrame, m_global);
    }

    if(m_encoder) {
        m_viewport.originX = (float)x;
        m_viewport.originY = (float)y;
        m_viewport.width = (float)width;
        m_viewport.height = (float)height;
        m_viewport.znear = (float)0.0f;
        m_viewport.zfar = (float)1.0f;

        m_encoder->setViewport(m_viewport);
        m_encoder->setScissorRect({(uint32_t)x, (uint32_t)y, (uint32_t)width, (uint32_t)height});
    }
}

void CommandBufferMt::enableScissor(int32_t x, int32_t y, int32_t width, int32_t height) {
    CommandBuffer::enableScissor(x, y, width, height);
    if(m_encoder) {
        ScissorRect rect = m_scissorStack.top();
        m_encoder->setScissorRect({(uint32_t)rect.x, (uint32_t)rect.y, (uint32_t)rect.width, (uint32_t)rect.height});
    }
}

void CommandBufferMt::disableScissor() {
    CommandBuffer::disableScissor();
    if(m_encoder) {
        if(m_scissorStack.empty()) {
            m_encoder->setScissorRect({(uint32_t)m_viewport.originX, (uint32_t)m_viewport.originY,
                                       (uint32_t)m_viewport.width, (uint32_t)m_viewport.height});
        } else {
            ScissorRect rect = m_scissorStack.top(); // Get previous
            m_encoder->setScissorRect({(uint32_t)rect.x, (uint32_t)rect.y, (uint32_t)rect.width, (uint32_t)rect.height});
        }
    }
}

void CommandBufferMt::flipResult() {
    m_global.params.w = 1.0f;
}
