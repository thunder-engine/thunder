#ifndef VULKANWINDOW_H
#define VULKANWINDOW_H

#include <QVulkanWindow>

class RenderVkSystem;

class VulkanWindowRenderer : public QVulkanWindowRenderer {
public:
    VulkanWindowRenderer(QVulkanWindow *w);

    void initSwapChainResources() override;
    void initResources() override;
    void releaseSwapChainResources() override;
    void releaseResources() override;

    void startNextFrame() override;

protected:
    QVulkanWindow *m_window;

};

class VulkanWindow : public QVulkanWindow {
    Q_OBJECT

public:
    void render();

signals:
    void draw();

protected:
    QVulkanWindowRenderer *createRenderer() override;

};

#endif // VULKANWINDOW_H
