#ifndef RENDERMTSYSTEM_H
#define RENDERMTSYSTEM_H

#include <cstdint>

#include <systems/rendersystem.h>

class Engine;

namespace MTK {
    class View;
}

class RenderMtSystem : public RenderSystem {
public:
    RenderMtSystem(Engine *engine);
    ~RenderMtSystem();

    bool init() override;

    void update(World *world) override;

    void setCurrentView(MTK::View *view);

    void createMetalWindow();

#ifdef SHARED_DEFINE
    QWindow *createRhiWindow(Viewport *viewport) override;
#endif

private:
    Engine *m_engine;

    MTK::View *m_currentView;

};

#endif // RENDERMTSYSTEM_H
