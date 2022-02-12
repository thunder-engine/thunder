#ifndef RENDERSYSTEM_H
#define RENDERSYSTEM_H

#include "engine.h"
#include "system.h"

class RenderSystemPrivate;

class Renderable;
class PostProcessSettings;

#if defined(SHARED_DEFINE)
class QWindow;
#endif

class ENGINE_EXPORT RenderSystem : public System {
public:
    RenderSystem();
    ~RenderSystem();

    bool init() override;

    void update(Scene *scene) override;

    int threadPolicy() const override;

    const char *name() const override;

    void composeComponent(Component *component) const override;

#if defined(SHARED_DEFINE)
    virtual QWindow *createRhiWindow() const;

    virtual vector<uint8_t> renderOffscreen(Scene *scene, int width, int height);
#endif

    static void atlasPageSize(int32_t &width, int32_t &height);

protected:
    static void setAtlasPageSize(int32_t width, int32_t height);

private:
    RenderSystemPrivate *p_ptr;

};

#endif // RENDERSYSTEM_H
