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

    void                        setRenderTarget             (const TargetBuffer &target, const RenderTexture *depth = nullptr);

    void                        setColor                    (const Vector4 &color);

    void                        setViewProjection           (const Matrix4 &view, const Matrix4 &projection);

    void                        setGlobalValue              (const char *name, const Variant &value);

    void                        setGlobalTexture            (const char *name, const Texture *value);

    void                        setViewport                 (uint32_t x, uint32_t y, uint32_t width, uint32_t height);

    Matrix4                     projection                  () const { return m_Projection; }

    Matrix4                     view                        () const { return m_View; }

    const Texture              *texture                     (const char *name) const;

protected:
    uint32_t                    handle                      (AMeshGL *mesh, uint32_t surface, uint32_t lod);

    void                        updateValues                ();

protected:
    AMaterialGL                 m_StaticVertex;

    uint32_t                    m_Static;

    uint32_t                    m_Pipeline;

    uint32_t                    m_Transform;

    int32_t                     m_ModelLocation;

    Matrix4                     m_View;

    Matrix4                     m_Model;

    Matrix4                     m_Projection;

    Vector4                     m_Color;

    VariantMap                  m_Uniforms;

    Material::TextureMap        m_Textures;

    AMaterialGL::ObjectMap      m_Objects;
};

#endif // COMMANDBUFFERGL_H
