#ifndef RENDERSYSTEM_H
#define RENDERSYSTEM_H

#include "engine.h"
#include "system.h"

class PipelineContext;
class Widget;
class BaseLight;
class Renderable;
class PostProcessVolume;

#if defined(SHARED_DEFINE)
class QWindow;
#endif

class ENGINE_EXPORT RenderSystem : public System {
public:
    RenderSystem();
    ~RenderSystem();

    bool init() override;

    void update(World *world) override;

    int threadPolicy() const override;

    void composeComponent(Component *component) const override;

    PipelineContext *pipelineContext() const;

#if defined(SHARED_DEFINE)
    virtual QWindow *createRhiWindow();

    virtual ByteArray renderOffscreen(World *world, int width, int height);
#endif

    void addRenderable(Renderable *renderable);
    void removeRenderable(Renderable *renderable);

    static list<Renderable *> &renderables();

    void addLight(BaseLight *light);
    void removeLight(BaseLight *light);

    static list<BaseLight *> &lights();

    void addPostProcessVolume(PostProcessVolume *volume);
    void removePostProcessVolume(PostProcessVolume *volume);

    static list<PostProcessVolume *> &postProcessVolumes();

protected:
    void setOffscreenMode(bool mode);
    bool isOffscreenMode() const;

private:
    static int32_t m_registered;

    static list<BaseLight *> m_lightComponents;
    static list<Renderable *> m_renderableComponents;
    static list<PostProcessVolume *> m_postProcessVolumes;

    bool m_offscreen;

    PipelineContext *m_pipelineContext;
};

#endif // RENDERSYSTEM_H
