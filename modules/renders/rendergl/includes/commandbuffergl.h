#ifndef COMMANDBUFFERGL_H
#define COMMANDBUFFERGL_H

#include <commandbuffer.h>

#include "resources/amaterialgl.h"
#include "resources/ameshgl.h"

class CommandBufferGL : public ICommandBuffer {
    A_OVERRIDE(CommandBufferGL, ICommandBuffer, System)

public:
    CommandBufferGL             ();

    ~CommandBufferGL            ();

    void                        clearRenderTarget           (bool clearColor = true, const Vector4 &color = Vector4(), bool clearDepth = true, float depth = 1.0f);

    void                        drawMesh                    (const Matrix4 &model, Mesh *mesh, uint32_t surface = 0, uint8_t layer = ICommandBuffer::DEFAULT, MaterialInstance *material = nullptr);

    void                        drawMeshInstanced           (const Matrix4 *models, uint32_t count, Mesh *mesh, uint32_t surface = 0, uint8_t layer = ICommandBuffer::DEFAULT, MaterialInstance *material = nullptr);

    void                        setRenderTarget             (const TargetBuffer &target, const RenderTexture *depth = nullptr);

    void                        setRenderTarget             (uint32_t target);

    void                        setColor                    (const Vector4 &color);

    void                        setViewProjection           (const Matrix4 &view, const Matrix4 &projection);

    void                        setGlobalValue              (const char *name, const Variant &value);

    void                        setGlobalTexture            (const char *name, const Texture *value);

    void                        setViewport                 (uint32_t x, uint32_t y, uint32_t width, uint32_t height);

    Matrix4                     projection                  () const { return m_Projection; }

    Matrix4                     view                        () const { return m_View; }

    const Texture              *texture                     (const char *name) const;

protected:
    void                        bindVao                     (AMeshGL *mesh, uint32_t surface, uint32_t lod, uint32_t instance = 0);

    void                        putUniforms                 (uint32_t fragment, MaterialInstance *instance);

protected:
    AMaterialGL                 m_StaticVertex;

    uint32_t                    m_Static;
    uint32_t                    m_Instanced;

    uint32_t                    m_Pipeline;

    int32_t                     m_ModelLocation;

    uint32_t                    m_InstanceBuffer;

    Matrix4                     m_View;

    Matrix4                     m_Model;

    Matrix4                     m_Projection;

    Vector4                     m_Color;

    VariantMap                  m_Uniforms;

    Material::TextureMap        m_Textures;

    AMaterialGL::ObjectMap      m_Objects;
};

#endif // COMMANDBUFFERGL_H
