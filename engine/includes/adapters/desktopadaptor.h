#ifndef DESKTOPADAPTOR_H
#define DESKTOPADAPTOR_H

#include "platformadaptor.h"

struct GLFWwindow;
struct GLFWmonitor;

class DesktopAdaptor : public PlatformAdaptor {
public:
    DesktopAdaptor();

    virtual ~DesktopAdaptor() {}

    bool init() override;

    bool start() override;

    void update() override;

    void loop() override;

    void destroy() override;

    bool key(Input::KeyCode code) const override;
    bool keyPressed(Input::KeyCode code) const override;
    bool keyReleased(Input::KeyCode code) const override;

    TString inputString() const override;

    Vector4 mousePosition() const override;
    Vector4 mouseDelta() const override;
    float mouseScrollDelta() const override;
    bool mouseButton(int button) const override;
    bool mousePressed(int button) const override;
    bool mouseReleased(int button) const override;

    void mouseLockCursor(bool lock) override;

    uint32_t screenWidth() const override;
    uint32_t screenHeight() const override;

    uint32_t joystickCount() const override;
    uint32_t joystickButtons(int index) const override;
    Vector4 joystickThumbs(int index) const override;
    Vector2 joystickTriggers(int index) const override;

    void *pluginLoad(const char *name) override;

    bool pluginUnload(void *plugin) override;

    void *pluginAddress(void *plugin, const TString &name) override;

    TString locationLocalDir() const override;

    void syncConfiguration(VariantMap &map) const override;

protected:
    static void toggleFullscreen(GLFWwindow *window);

    static void keyCallback(GLFWwindow *, int, int, int, int);

    static void charCallback(GLFWwindow *, unsigned int);

    static void buttonCallback(GLFWwindow*,int, int, int);

    static void scrollCallback(GLFWwindow *, double, double yoffset);

    static void cursorPositionCallback(GLFWwindow *, double xpos, double ypos);

    static void errorCallback(int error, const char *description);

protected:
    GLFWwindow *m_pWindow;
    GLFWmonitor *m_pMonitor;

    bool m_noOpenGL;

    static Vector4 s_mousePosition;
    static Vector4 s_oldMousePosition;

    static float s_mouseScrollDelta;

    static int32_t s_width;
    static int32_t s_height;
    static bool s_windowed;
    static bool s_vSync;
};

#endif // DESKTOPADAPTOR_H
