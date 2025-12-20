#ifndef EDITORPLATFORM_H
#define EDITORPLATFORM_H

#include <QHash>
#include <QSize>
#include <QPoint>

#include <adapters/platformadaptor.h>

#define NONE 0
#define RELEASE 1
#define PRESS 2
#define REPEAT 3

class Engine;
class Camera;
class QKeyEvent;

class ENGINE_EXPORT EditorPlatform : public PlatformAdaptor {
public:
    static EditorPlatform &instance();

    void setImportPath(const TString &path);

    void setScreenSize(const QSize &size);

    bool isMouseLocked() const;
    void setMousePosition(const QPoint &position);
    void setMouseDelta(const QPoint &position);
    void setMouseScrollDelta(float delta);
    void setMouseButtons(int button, int state);

    void setKeys(QKeyEvent *ev, bool release);

    void update() override;
    void reset();

protected:
    EditorPlatform();

    bool init() override;

    bool start() override { return true; }

    void destroy() override {}

    bool key(Input::KeyCode) const override;
    bool keyPressed(Input::KeyCode) const override;
    bool keyReleased(Input::KeyCode) const override;

    TString inputString() const override;

    bool mouseButton(int) const override;
    bool mousePressed(int) const override;
    bool mouseReleased(int) const override;

    Vector4 mousePosition() const override;
    void mouseLockCursor(bool lock) override;

    Vector4 mouseDelta() const override;
    float mouseScrollDelta() const override;

    uint32_t screenWidth() const override;
    uint32_t screenHeight() const override;

    TString locationLocalDir() const override;

    void syncConfiguration(VariantMap &map) const override;

protected:
    QHash<int32_t, int32_t> m_keys;
    QHash<int32_t, int32_t> m_mouseButtons;

    QSize m_screenSize;

    TString m_inputString;

    Vector4 m_mouseDelta;

    Vector4 m_mousePosition;

    float m_mouseScrollDelta;

    bool m_mouseLock;

};

#endif // EDITORPLATFORM_H
