#include "editor/vulkanwindow.h"

#include <vulkan/vulkan.h>

#include <QVulkanInstance>
#include <QPlatformSurfaceEvent>

#include "wrappervk.h"

ThunderVulkanWindow::ThunderVulkanWindow(RenderVkSystem *system) :
        m_system(system),
        m_status(StatusUninitialized) {

    setSurfaceType(QSurface::VulkanSurface);

    m_system->setCurrentSurface(m_surface);
}

void ThunderVulkanWindow::exposeEvent(QExposeEvent *) {
    if(isExposed()) {
        ensureStarted();
    }
}

bool ThunderVulkanWindow::event(QEvent *e) {
    switch(e->type()) {
        case QEvent::UpdateRequest: {
            if(isVisible()) {
                if(m_surface.beginFrame(width(), height())) {
                    emit draw();

                    m_surface.endFrame();
                }

                requestUpdate();
            }
        } break;
        case QEvent::PlatformSurface: {
            if(static_cast<QPlatformSurfaceEvent *>(e)->surfaceEventType() == QPlatformSurfaceEvent::SurfaceAboutToBeDestroyed) {
                m_surface.releaseSwapChain();
            }
        } break;
        default: break;
    }

    return QWindow::event(e);
}

void ThunderVulkanWindow::ensureStarted() {
    if(m_status == StatusFailRetry) {
        m_status = StatusUninitialized;
    }

    if(m_status == StatusUninitialized) {
        m_surface.m_nativeSurface = QVulkanInstance::surfaceForWindow(this);
        if(m_surface.m_nativeSurface == VK_NULL_HANDLE) {
            m_status = StatusFailRetry;
            return;
        }

        m_status = StatusDeviceReady;
        m_surface.selectFormats();
    }

    if(m_status == StatusDeviceReady) {
        m_surface.recreateSwapChain();

        m_status = StatusReady;
    }

    if(m_status == StatusReady) {
        requestUpdate();
    }
}
