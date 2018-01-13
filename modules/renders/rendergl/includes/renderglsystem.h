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

    void                        drawBillboard               (const Vector3 &position, const Vector2 &size, Texture &image);

    void                        drawPath                    (const Vector3List &points);

    void                        setColor                    (const Vector4 &color);

protected:
    APipeline                  *m_pPipeline;

};

#endif // RENDERGLSYSTEM_H
