#ifndef SCENEVIEW_H
#define SCENEVIEW_H

#include <QWidget>

#include <adapters/platformadaptor.h>

class Engine;
class System;
class Scene;

class QGamepad;

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

    bool key(Input::KeyCode) const override;
    bool keyPressed(Input::KeyCode) const override;
    bool keyReleased(Input::KeyCode) const override;

    string inputString() const override;

    bool mouseButton(int) const override;
    bool mousePressed(int) const override;
    bool mouseReleased(int) const override;

    Vector4 mousePosition() const override;
    void mouseLockCursor(bool lock) override;

    Vector4 mouseDelta() const override;

    uint32_t screenWidth() const override;
    uint32_t screenHeight() const override;

    uint32_t joystickCount() const override;
    uint32_t joystickButtons(int) const override;
    Vector4 joystickThumbs(int) const override;
    Vector2 joystickTriggers(int) const override;

    string locationLocalDir() const override;

    bool eventFilter(QObject *object, QEvent *event) override;

private slots:
    void onDraw();

    void onGamepadConnected(bool value);

protected:
    QWindow *m_rhiWindow;

    Engine *m_engine;

    QGamepad *m_gamepad;

    QHash<int32_t, int32_t> m_keys;
    QHash<int32_t, int32_t> m_mouseButtons;

    string m_inputString;

    QPoint m_saved;

    Vector4 m_mouseDelta;

    bool m_mouseLock;

};

#endif // SCENEVIEW_H
