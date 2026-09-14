/*
    This file is part of Thunder Engine.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
#include "editor/vulkanwindow.h"

#include <viewport/viewport.h>

#include <vulkan/vulkan.h>

#include <QVulkanInstance>
#include <QPlatformSurfaceEvent>

ThunderVulkanWindow::ThunderVulkanWindow(Viewport *viewport, RenderVkSystem *system) :
        m_viewport(viewport),
        m_system(system),
        m_status(StatusUninitialized) {

    setSurfaceType(QSurface::VulkanSurface);
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
                m_system->setCurrentSurface(m_surface);
                if(m_surface.beginFrame(width(), height())) {
                    m_viewport->onDraw();
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
