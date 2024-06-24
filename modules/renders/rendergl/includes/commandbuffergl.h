#ifndef COMMANDBUFFERGL_H
#define COMMANDBUFFERGL_H

#include <commandbuffer.h>

#define VERTEX_ATRIB    0
#define UV0_ATRIB       1
#define COLOR_ATRIB     2
#define NORMAL_ATRIB    3
#define TANGENT_ATRIB   4

#define BONES_ATRIB     5
#define WEIGHTS_ATRIB   6

class CommandBufferGL : public CommandBuffer {
    A_OVERRIDE(CommandBufferGL, CommandBuffer, System)

public:
    CommandBufferGL();

    void begin();

    void clearRenderTarget(bool clearColor = true, const Vector4 &color = Vector4(0.0f), bool clearDepth = true, float depth = 1.0f) override;

    void dispatchCompute(ComputeInstance *shader, int32_t groupsX, int32_t groupsY, int32_t groupsZ) override;

    void drawMesh(Mesh *mesh, uint32_t sub, uint32_t layer, MaterialInstance &instance) override;

    void setRenderTarget(RenderTarget *target, uint32_t level = 0) override;

    void setViewProjection(const Matrix4 &view, const Matrix4 &projection) override;

    void setViewport(int32_t x, int32_t y, int32_t width, int32_t height) override;

    void enableScissor(int32_t x, int32_t y, int32_t width, int32_t height) override;

    void disableScissor() override;

    void beginDebugMarker(const char *name) override;
    void endDebugMarker() override;

    static void setObjectName(int32_t type, int32_t id, const string &name);

protected:
    uint32_t m_globalUbo;


};

#endif // COMMANDBUFFERGL_H
