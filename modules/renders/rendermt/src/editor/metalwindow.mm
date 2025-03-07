#include "editor/metalwindow.h"

#include <systems/resourcesystem.h>

#include <QPlatformSurfaceEvent>

#include "viewdelegate.h"

@import MetalKit;

ThunderMetalWindow::ThunderMetalWindow(RenderMtSystem *system) :
        m_system(system),
        m_delegate(nullptr) {

    setSurfaceType(QSurface::MetalSurface);
}

void ThunderMetalWindow::exposeEvent(QExposeEvent *) {
    if(m_delegate) {
        return;
    }

    //NSView *view = reinterpret_cast<NSView *>(winId());
    //MTK::View *MtkView = reinterpret_cast<MTK::View *>(winId());

    //m_delegate = new ViewDelegate(m_system);
    //view->setDelegate(m_delegate);
}

bool ThunderMetalWindow::event(QEvent *e) {
    switch(e->type()) {
        case QEvent::UpdateRequest: {
            if(isVisible()) {
                //if(m_surface.beginFrame(width(), height())) {
                    emit draw();
                    //m_surface.endFrame();
                //}

                requestUpdate();
            }
        } break;
        case QEvent::PlatformSurface: {
            if(static_cast<QPlatformSurfaceEvent *>(e)->surfaceEventType() == QPlatformSurfaceEvent::SurfaceAboutToBeDestroyed) {
                // release
            }
        } break;
        default: break;
    }

    return QWindow::event(e);
}
