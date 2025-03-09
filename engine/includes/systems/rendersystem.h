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
class Viewport;
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

    void addRenderable(Renderable *renderable);
    void removeRenderable(Renderable *renderable);

    static std::list<Renderable *> &renderables();

    void addLight(BaseLight *light);
    void removeLight(BaseLight *light);

    static std::list<BaseLight *> &lights();

    void addPostProcessVolume(PostProcessVolume *volume);
    void removePostProcessVolume(PostProcessVolume *volume);

    static std::list<PostProcessVolume *> &postProcessVolumes();

#if defined(SHARED_DEFINE)
    virtual QWindow *createRhiWindow(Viewport *viewport);
#endif

private:
    static int32_t m_registered;

    static std::list<BaseLight *> m_lightComponents;
    static std::list<Renderable *> m_renderableComponents;
    static std::list<PostProcessVolume *> m_postProcessVolumes;

    PipelineContext *m_pipelineContext;
};

#endif // RENDERSYSTEM_H
