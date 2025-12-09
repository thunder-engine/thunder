#ifndef COMMANDBUFFERGL_H
#define COMMANDBUFFERGL_H

#include <commandbuffer.h>

class CommandBufferGL : public CommandBuffer {
    A_OBJECT_OVERRIDE(CommandBufferGL, CommandBuffer, System)

public:
    CommandBufferGL();

    void dispatchCompute(ComputeInstance *shader, int32_t groupsX, int32_t groupsY, int32_t groupsZ) override;

    void drawMesh(Mesh *mesh, uint32_t sub, uint32_t layer, MaterialInstance &instance) override;

    void setRenderTarget(RenderTarget *target, uint32_t level = 0) override;

    void setViewport(int32_t x, int32_t y, int32_t width, int32_t height) override;

    void enableScissor(int32_t x, int32_t y, int32_t width, int32_t height) override;

    void disableScissor() override;

    void beginDebugMarker(const TString &name) override;
    void endDebugMarker() override;

    static void setObjectName(int32_t type, int32_t id, const TString &name);

};

#endif // COMMANDBUFFERGL_H
