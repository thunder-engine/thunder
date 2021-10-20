#ifndef SCENEVIEW_H
#define SCENEVIEW_H

#include <QOpenGLWidget>

#include <adapters/platformadaptor.h>

class Engine;
class System;
class Scene;

class QOffscreenSurface;
class QOpenGLFramebufferObject;

class SceneView : public QWidget, public PlatformAdaptor {
    Q_OBJECT
public:
    SceneView(QWidget *parent = nullptr);
    ~SceneView() override;

    void setEngine(Engine *engine);

protected:
    bool init() override { return true; }

    void update() override;

    bool start() override { return true; }

    void stop() override {}

    void destroy() override {}

    bool isValid() override { return true; }

    bool key(Input::KeyCode) override;
    bool keyPressed(Input::KeyCode) override;
    bool keyReleased(Input::KeyCode) override;

    string inputString() override;

    bool mouseButton(Input::MouseButton) override;
    bool mousePressed(Input::MouseButton) override;
    bool mouseReleased(Input::MouseButton) override;

    Vector4 mousePosition() override;
    void setMousePosition(int32_t x, int32_t y) override;

    Vector4 mouseDelta() override;

    uint32_t screenWidth() override;
    uint32_t screenHeight() override;

    uint32_t joystickCount() override;
    uint32_t joystickButtons(uint32_t) override;
    Vector4 joystickThumbs(uint32_t) override;
    Vector2 joystickTriggers(uint32_t) override;

    void *pluginLoad(const char *) override;
    bool pluginUnload(void *) override;
    void *pluginAddress(void *, const string &) override;

    string locationLocalDir() override;

    bool eventFilter(QObject *object, QEvent *event) override;

private slots:
    void onDraw();

protected:
    QWindow *m_pRHIWindow;

    Engine *m_pEngine;

    unordered_map<int32_t, int32_t> m_Keys;
    unordered_map<int32_t, int32_t> m_MouseButtons;

    string m_inputString;

};

#endif // SCENEVIEW_H
