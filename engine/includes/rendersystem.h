#ifndef RENDERSYSTEM_H
#define RENDERSYSTEM_H

#include "system.h"

#include <amath.h>
#include "resources/texture.h"

class Camera;
class Mesh;
class MaterialInstance;

class NEXT_LIBRARY_EXPORT IRenderSystem : public ISystem {
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
    IRenderSystem               (Engine *engine);

    virtual void                clearRenderTarget           (bool clearColor, const Vector4 &color, bool clearDepth, float depth) = 0;

    virtual void                drawMesh                    (const Matrix4 &model, Mesh *mesh, uint32_t surface = 0, uint8_t layer = IRenderSystem::DEFAULT, MaterialInstance *material = nullptr) = 0;

    virtual void                setColor                    (const Vector4 &color) = 0;

    virtual void                setCamera                   (const Camera &camera) = 0;

    virtual void                setRenderTarget             (uint8_t numberColors, const Texture *colors, uint8_t numberDepth, const Texture *depth) = 0;

};

#endif // RENDERSYSTEM_H
