#ifndef RENDERSYSTEM_H
#define RENDERSYSTEM_H

#include "engine.h"
#include "system.h"

class RenderSystemPrivate;

class PipelineContext;

#if defined(SHARED_DEFINE)
class QWindow;
#endif

class ENGINE_EXPORT RenderSystem : public System {
public:
    RenderSystem();
    ~RenderSystem();

    bool init() override;

    void update(SceneGraph *sceneGraph) override;

    int threadPolicy() const override;

    const char *name() const override;

    void composeComponent(Component *component) const override;

    PipelineContext *pipelineContext() const;

#if defined(SHARED_DEFINE)
    virtual QWindow *createRhiWindow() const;

    virtual ByteArray renderOffscreen(SceneGraph *sceneGraph, int width, int height);
#endif

protected:
    void setOffscreenMode(bool mode);
    bool isOffscreenMode() const;

private:
    RenderSystemPrivate *p_ptr;

};

#endif // RENDERSYSTEM_H
