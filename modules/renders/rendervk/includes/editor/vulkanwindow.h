#ifndef VULKANWINDOW_H
#define VULKANWINDOW_H

#include <QVulkanWindow>

class TriangleRenderer : public QVulkanWindowRenderer {
public:
    TriangleRenderer(QVulkanWindow *w);

    void initSwapChainResources() override;
    void initResources() override;
    void releaseSwapChainResources() override;
    void releaseResources() override;

    void startNextFrame() override;

protected:
    QVulkanWindow *m_window;
    QVulkanDeviceFunctions *m_devFuncs;

};

class VulkanWindow : public QVulkanWindow {
    Q_OBJECT

signals:
    void draw();

protected:
    QVulkanWindowRenderer *createRenderer() override;

};

#endif // VULKANWINDOW_H
