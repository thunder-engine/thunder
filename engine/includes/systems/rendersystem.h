#ifndef RENDERSYSTEM_H
#define RENDERSYSTEM_H

#include "engine.h"
#include "system.h"

class PipelineContext;
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
    void setPipelineContext(PipelineContext *context);

#if defined(SHARED_DEFINE)
    virtual QWindow *createRhiWindow(Viewport *viewport);
#endif

    static void *windowHandle();
    static void setWindowHandle(void *handle);

protected:
    static void *m_windowHandle;

private:
    PipelineContext *m_pipelineContext;

    bool m_frameDirty;
};

#endif // RENDERSYSTEM_H
