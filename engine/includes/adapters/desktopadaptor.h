#ifndef DESKTOPAADAPTOR_H
#define DESKTOPAADAPTOR_H

#include "iplatformadaptor.h"

struct GLFWwindow;
struct GLFWmonitor;

class DesktopAdaptor : public IPlatformAdaptor {
public:
    DesktopAdaptor              (Engine *engine);

    virtual ~DesktopAdaptor     () {}

    bool                        init                        ();

    void                        update                      ();

    bool                        start                       ();

    void                        stop                        ();

    void                        destroy                     ();

    bool                        isValid                     ();

    bool                        keyPressed                  (Input::KeyCode code);
    bool                        keyReleased                 (Input::KeyCode code);

    Vector4                     mousePosition               ();

    Vector4                     mouseDelta                  ();

    uint32_t                    mouseButtons                ();

    bool                        mousePressed                (Input::MouseButton button);
    bool                        mouseReleased               (Input::MouseButton button);

    uint32_t                    screenWidth                 ();

    uint32_t                    screenHeight                ();

    void                        setMousePosition            (int32_t x, int32_t y);

    uint32_t                    joystickCount               ();

    uint32_t                    joystickButtons             (uint32_t index);

    Vector4                     joystickThumbs              (uint32_t index);

    Vector2                     joystickTriggers            (uint32_t index);

    void                       *pluginLoad                  (const char *name);

    bool                        pluginUnload                (void *plugin);

    void                       *pluginAddress               (void *plugin, const string &name);

    string                      locationLocalDir            ();

    void                        syncConfiguration           (VariantMap &map) const;

protected:
    static void                 scrollCallback              (GLFWwindow *, double, double yoffset);

    static void                 cursorPositionCallback      (GLFWwindow *, double xpos, double ypos);

    static void                 errorCallback               (int error, const char *description);

protected:
    GLFWwindow                 *m_pWindow;

    GLFWmonitor                *m_pMonitor;

    uint8_t                     m_MouseButtons;
    uint8_t                     m_LastMouseButtons;

    static Vector4              s_MousePosition;

    static Vector4              s_OldMousePosition;

    static int32_t              s_Width;
    static int32_t              s_Height;
    static bool                 s_Windowed;
};

#endif // DESKTOPAADAPTOR_H
