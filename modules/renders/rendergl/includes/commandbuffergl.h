#ifndef COMMANDBUFFERGL_H
#define COMMANDBUFFERGL_H

#include <commandbuffer.h>

#include "resources/amaterialgl.h"
#include "resources/ameshgl.h"

class CommandBufferGL : public ICommandBuffer {
public:
    CommandBufferGL             ();

    void                        clearRenderTarget           (bool clearColor = true, const Vector4 &color = Vector4(), bool clearDepth = true, float depth = 1.0f);

    void                        drawMesh                    (const Matrix4 &model, Mesh *mesh, uint32_t surface = 0, uint8_t layer = ICommandBuffer::DEFAULT, MaterialInstance *material = nullptr);

    void                        setRenderTarget             (uint8_t numberColors, const Texture *colors, const Texture *depth);

    void                        setColor                    (const Vector4 &color);

    void                        setViewProjection           (const Matrix4 &view, const Matrix4 &projection);

    void                        setGlobalValue              (const char *name, const AVariant &value);

    Matrix4                     projection                  () const { return m_Projection; }

    Matrix4                     modelView                   () const { return m_View * m_Model; }

protected:
    void                        setShaderParams             (uint32_t program);

protected:
    Vector4                     m_Color;

    Matrix4                     m_View;
    Matrix4                     m_Model;
    Matrix4                     m_Projection;

    AVariantMap                 m_Uniforms;

};

#endif // COMMANDBUFFERGL_H
