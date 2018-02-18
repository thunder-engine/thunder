#ifndef RENDERGLSYSTEM_H
#define RENDERGLSYSTEM_H

#include <cstdint>

#include <rendersystem.h>

class APipeline;

class RenderGLSystem : public IRenderSystem {
public:
    RenderGLSystem             (Engine *engine);
    ~RenderGLSystem            ();

    bool                        init                        ();

    const char                 *name                        () const;

    void                        update                      (Scene &scene, uint32_t resource = 0);

    void                        overrideController          (IController *controller);

    void                        clearRenderTarget           (bool clearColor, const Vector4 &color, bool clearDepth, float depth);

    void                        drawMesh                    (const Matrix4 &model, Mesh *mesh, uint32_t surface = 0, uint8_t layer = IRenderSystem::DEFAULT, MaterialInstance *material = nullptr);

    void                        setColor                    (const Vector4 &color);

    void                        setCamera                   (const Camera &camera);

    void                        setRenderTarget             (uint8_t numberColors, const Texture *colors, uint8_t numberDepth, const Texture *depth);

protected:
    APipeline                  *m_pPipeline;

};

#endif // RENDERGLSYSTEM_H
