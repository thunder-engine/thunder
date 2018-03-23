#ifndef COMMANDBUFFER_H
#define COMMANDBUFFER_H

#include <object.h>

class Texture;
class Mesh;

class Camera;
class MaterialInstance;

class NEXT_LIBRARY_EXPORT ICommandBuffer {
public:
    enum LayerTypes {
        DEFAULT     = (1<<0),
        RAYCAST     = (1<<1),
        SHADOWCAST  = (1<<2),
        LIGHT       = (1<<3),
        TRANSLUCENT = (1<<4),
        UI          = (1<<6)
    };

public:
    virtual void                clearRenderTarget           (bool clearColor = true, const Vector4 &color = Vector4(), bool clearDepth = true, float depth = 1.0f);

    virtual void                drawMesh                    (const Matrix4 &model, Mesh *mesh, uint32_t surface = 0, uint8_t layer = ICommandBuffer::DEFAULT, MaterialInstance *material = nullptr);

    virtual void                setRenderTarget             (uint8_t numberColors, const Texture *colors, const Texture *depth);

    virtual void                setColor                    (uint32_t id);

    virtual void                setColor                    (const Vector4 &color);

    virtual void                setViewProjection           (const Matrix4 &view, const Matrix4 &projection);

    virtual void                setGlobalValue              (const char *name, const Variant &value);

    virtual void                setViewport                 (uint32_t x, uint32_t y, uint32_t width, uint32_t height);

    virtual Matrix4             projection                  () const;

    virtual Matrix4             modelView                   () const;

protected:
    static void                 setHandler                  (ICommandBuffer *handler);

};

#endif // COMMANDBUFFER_H
