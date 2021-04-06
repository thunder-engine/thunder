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

    list<Renderable *> &renderable() const;

    list<Renderable *> &lights() const;

    list<PostProcessSettings *> &postPcessSettings() const;

    static void atlasPageSize(int32_t &width, int32_t &height);

protected:
    static void setAtlasPageSize(int32_t width, int32_t height);

    Object *instantiateObject(const MetaObject *meta) override;

    void removeObject(Object *object) override;

private:
    RenderSystemPrivate *p_ptr;

};

#endif // RENDERSYSTEM_H
