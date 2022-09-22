#ifndef EDITORPLATFORM_H
#define EDITORPLATFORM_H

#include <QWidget>

#include <adapters/platformadaptor.h>

#define NONE 0
#define RELEASE 1
#define PRESS 2
#define REPEAT 3

class Engine;
class Camera;

class QGamepad;

class ENGINE_EXPORT EditorPlatform : public PlatformAdaptor {
public:
    static EditorPlatform &instance();

    void setScreenSize(const QSize &size);

    bool isMouseLocked() const;
    void setMousePosition(const Vector2 &position);
    void setMouseButtons(int button, int state);

    void setKeys(int key, const QString &text, bool release, bool repeat);

    void update() override;

protected:
    EditorPlatform();
    ~EditorPlatform();

    bool init() override;

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

private slots:
    void onGamepadConnected(bool value);

protected:
    QGamepad *m_gamepad;

    QHash<int32_t, int32_t> m_keys;
    QHash<int32_t, int32_t> m_mouseButtons;

    Vector4 m_saved;

    QSize m_screenSize;

    string m_inputString;

    Vector4 m_mouseDelta;

    Vector4 m_mousePosition;

    bool m_mouseLock;

};

#endif // EDITORPLATFORM_H
