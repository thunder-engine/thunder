#include "viewdelegate.h"

#include "rendermtsystem.h"

#include <timer.h>

#if defined(SHARED_DEFINE)
#include "editor/viewport/viewport.h"
#endif

ViewDelegate::ViewDelegate(RenderMtSystem *system, Viewport *viewport) :
        m_render(system),
        m_viewport(viewport) {

}

void ViewDelegate::drawInMTKView(MTK::View *view) {
    m_render->setCurrentView(view);

    // Render cycle here
#if defined(SHARED_DEFINE)
    m_viewport->onDraw();
#else
    Timer::update();
    Engine::update();
#endif
}
