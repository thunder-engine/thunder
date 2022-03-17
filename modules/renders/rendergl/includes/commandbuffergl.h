#ifndef COMMANDBUFFERGL_H
#define COMMANDBUFFERGL_H

#include <commandbuffer.h>

#include "resources/materialgl.h"
#include "resources/meshgl.h"

#define VERTEX_ATRIB    0
#define UV0_ATRIB       1
#define NORMAL_ATRIB    2
#define TANGENT_ATRIB   3
#define COLOR_ATRIB     4

#define UV1_ATRIB       5
#define BONES_ATRIB     6
#define WEIGHTS_ATRIB   7

#define INSTANCE_ATRIB  8

class CommandBufferGL : public CommandBuffer {
    A_OVERRIDE(CommandBufferGL, CommandBuffer, System)

public:
    CommandBufferGL();

    ~CommandBufferGL() override;

    void begin();

    void clearRenderTarget(bool clearColor = true, const Vector4 &color = Vector4(0.0f), bool clearDepth = true, float depth = 1.0f) override;

    void drawMesh(const Matrix4 &model, Mesh *mesh, uint32_t sub, uint32_t layer = CommandBuffer::DEFAULT, MaterialInstance *material = nullptr) override;

    void drawMeshInstanced(const Matrix4 *models, uint32_t count, Mesh *mesh, uint32_t sub, uint32_t layer = CommandBuffer::DEFAULT, MaterialInstance *material = nullptr) override;

    void setRenderTarget(RenderTarget *target, uint32_t level = 0) override;

    void resetViewProjection() override;

    void setViewProjection(const Matrix4 &view, const Matrix4 &projection) override;

    void setGlobalValue(const char *name, const Variant &value) override;

    void setGlobalTexture(const char *name, Texture *texture) override;

    void setViewport(int32_t x, int32_t y, int32_t width, int32_t height) override;

    void enableScissor(int32_t x, int32_t y, int32_t width, int32_t height) override;

    void disableScissor() override;

    Texture *texture(const char *name) const override;

    void finish() override;

    const VariantMap &params() const;

protected:
    VariantMap m_uniforms;
    Material::TextureList m_textures;

    uint32_t m_globalUbo;
    uint32_t m_localUbo;

};

#endif // COMMANDBUFFERGL_H
