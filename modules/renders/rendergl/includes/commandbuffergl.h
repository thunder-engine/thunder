#ifndef COMMANDBUFFERGL_H
#define COMMANDBUFFERGL_H

#include <commandbuffer.h>

class TextureGL;

class CommandBufferGL : public CommandBuffer {
    A_OBJECT_OVERRIDE(CommandBufferGL, CommandBuffer, System)

public:
    CommandBufferGL();
    ~CommandBufferGL();

    static void setObjectName(int32_t type, int32_t id, const TString &name);

    void bindTexture(uint32_t index, TextureGL *texture);

protected:
    void dispatchCompute(ComputeInstance &shader, int32_t groupsX, int32_t groupsY, int32_t groupsZ) override;

    void drawMesh(Mesh *mesh, uint32_t sub, uint32_t layer, MaterialInstance &instance) override;

    void setRenderTarget(RenderTarget *target, uint32_t level = 0) override;

    void setViewport(int32_t x, int32_t y, int32_t width, int32_t height) override;

    void enableScissor(int32_t x, int32_t y, int32_t width, int32_t height) override;

    void disableScissor() override;

    void beginDebugMarker(const TString &name) override;
    void endDebugMarker() override;

    void setViewProjection(const Matrix4 &viewProjection) override;

    void updateGlobal();

protected:
    uint32_t m_globalBuffer;

    uint32_t m_textures[32];

};

#endif // COMMANDBUFFERGL_H
