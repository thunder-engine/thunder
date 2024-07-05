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

    static std::list<Renderable *> &renderables();

    void addLight(BaseLight *light);
    void removeLight(BaseLight *light);

    static std::list<BaseLight *> &lights();

    void addPostProcessVolume(PostProcessVolume *volume);
    void removePostProcessVolume(PostProcessVolume *volume);

    static std::list<PostProcessVolume *> &postProcessVolumes();

protected:
    void setOffscreenMode(bool mode);
    bool isOffscreenMode() const;

private:
    static int32_t m_registered;

    static std::list<BaseLight *> m_lightComponents;
    static std::list<Renderable *> m_renderableComponents;
    static std::list<PostProcessVolume *> m_postProcessVolumes;

    bool m_offscreen;

    PipelineContext *m_pipelineContext;
};

#endif // RENDERSYSTEM_H
