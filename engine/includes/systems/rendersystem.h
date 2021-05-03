#ifndef RENDERSYSTEM_H
#define RENDERSYSTEM_H

#include "system.h"

class RenderSystemPrivate;

class Renderable;
class PostProcessSettings;

class NEXT_LIBRARY_EXPORT RenderSystem : public System {
public:
    RenderSystem();
    ~RenderSystem();

    bool init() override;

    void update(Scene *scene) override;

    int threadPolicy() const override;

    const char *name() const override;

    void composeComponent(Component *component) const override;

    static void atlasPageSize(int32_t &width, int32_t &height);

protected:
    static void setAtlasPageSize(int32_t width, int32_t height);

private:
    RenderSystemPrivate *p_ptr;

};

#endif // RENDERSYSTEM_H
