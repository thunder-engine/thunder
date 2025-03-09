#ifndef COMMANDBUFFERMT_H
#define COMMANDBUFFERMT_H

#include <commandbuffer.h>

#include "wrappermt.h"

class RenderTargetMt;

class CommandBufferMt : public CommandBuffer {
    A_OVERRIDE(CommandBufferMt, CommandBuffer, System)

public:
    CommandBufferMt();

    void begin(MTL::CommandBuffer *cmd);

    void end();

    MTL::RenderCommandEncoder *encoder() const;

    RenderTargetMt *currentRenderTarget() const;

    void dispatchCompute(ComputeInstance *shader, int32_t groupsX, int32_t groupsY, int32_t groupsZ) override;

    void drawMesh(Mesh *mesh, uint32_t sub, uint32_t layer, MaterialInstance &instance) override;

    void setRenderTarget(RenderTarget *target, uint32_t level = 0) override;

    void setViewport(int32_t x, int32_t y, int32_t width, int32_t height) override;

    void enableScissor(int32_t x, int32_t y, int32_t width, int32_t height) override;

    void disableScissor() override;

    void beginDebugMarker(const char *name) override;
    void endDebugMarker() override;

protected:
    MTL::CommandBuffer *m_commandBuffer;

    MTL::RenderCommandEncoder *m_encoder;

    RenderTargetMt *m_currentTarget;

};

#endif // COMMANDBUFFERMT_H
