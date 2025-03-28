#ifndef VULKANWINDOW_H
#define VULKANWINDOW_H

#include "rendervksystem.h"

#include <QWindow>

class ThunderVulkanWindow : public QWindow {
    Q_OBJECT

    enum Status {
        StatusUninitialized,
        StatusFailRetry,
        StatusDeviceReady,
        StatusReady
    };

public:
    explicit ThunderVulkanWindow(Viewport *viewport, RenderVkSystem *system);

private:
    void exposeEvent(QExposeEvent *) override;
    bool event(QEvent *) override;

    void ensureStarted();

private:
    Viewport *m_viewport;

    RenderVkSystem *m_system;

    SurfaceVk m_surface;

    Status m_status;

};

#endif // VULKANWINDOW_H
