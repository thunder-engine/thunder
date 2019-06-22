#ifndef COMMANDBUFFERGL_H
#define COMMANDBUFFERGL_H

#include <commandbuffer.h>

#include "resources/amaterialgl.h"
#include "resources/ameshgl.h"

#define VERTEX_ATRIB    0
#define NORMAL_ATRIB    1
#define TANGENT_ATRIB   2
#define COLOR_ATRIB     3
#define UV0_ATRIB       4

#define INSTANCE_ATRIB  5

class CommandBufferGL : public ICommandBuffer {
    A_OVERRIDE(CommandBufferGL, ICommandBuffer, System)

public:
    CommandBufferGL             ();

    ~CommandBufferGL            ();

    void                        clearRenderTarget           (bool clearColor = true, const Vector4 &color = Vector4(), bool clearDepth = true, float depth = 1.0f);

    void                        drawMesh                    (const Matrix4 &model, Mesh *mesh, uint32_t layer = ICommandBuffer::DEFAULT, MaterialInstance *material = nullptr);

    void                        drawMeshInstanced           (const Matrix4 *models, uint32_t count, Mesh *mesh, uint32_t layer = ICommandBuffer::DEFAULT, MaterialInstance *material = nullptr, bool particle = false);

    void                        setRenderTarget             (const TargetBuffer &target, const RenderTexture *depth = nullptr);

    void                        setRenderTarget             (uint32_t target);

    void                        setColor                    (const Vector4 &color);

    void                        resetViewProjection         ();

    void                        setViewProjection           (const Matrix4 &view, const Matrix4 &projection);

    void                        setGlobalValue              (const char *name, const Variant &value);

    void                        setGlobalTexture            (const char *name, const Texture *value);

    void                        setViewport                 (int32_t x, int32_t y, int32_t width, int32_t height);

    Matrix4                     projection                  () const { return m_Projection; }

    Matrix4                     view                        () const { return m_View; }

    const Texture              *texture                     (const char *name) const;

protected:
    void                        putUniforms                 (uint32_t program, MaterialInstance *instance);

protected:
    AMaterialGL                 m_StaticVertex;

    Matrix4                     m_View;

    Matrix4                     m_Projection;

    Matrix4                     m_Model;

    Vector4                     m_Color;

    VariantMap                  m_Uniforms;

    Material::TextureMap        m_Textures;

    Matrix4                     m_SaveView;

    Matrix4                     m_SaveProjection;
};

#endif // COMMANDBUFFERGL_H
