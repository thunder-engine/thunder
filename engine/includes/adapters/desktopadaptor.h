#ifndef DESKTOPADAPTOR_H
#define DESKTOPADAPTOR_H

#include "platformadaptor.h"

struct GLFWwindow;
struct GLFWmonitor;

class DesktopAdaptor : public PlatformAdaptor {
public:
    DesktopAdaptor              (Engine *engine);

    virtual ~DesktopAdaptor     () {}

    bool                        init                        ();

    void                        update                      ();

    bool                        start                       ();

    void                        stop                        ();

    void                        destroy                     ();

    bool                        isValid                     ();

    bool                        key                         (Input::KeyCode code);
    bool                        keyPressed                  (Input::KeyCode code);
    bool                        keyReleased                 (Input::KeyCode code);

    string                      inputString                 ();

    Vector4                     mousePosition               ();
    Vector4                     mouseDelta                  ();
    bool                        mouseButton                 (Input::MouseButton button);
    bool                        mousePressed                (Input::MouseButton button);
    bool                        mouseReleased               (Input::MouseButton button);

    void                        setMousePosition            (int32_t x, int32_t y);

    uint32_t                    screenWidth                 ();
    uint32_t                    screenHeight                ();

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
    static void                 keyCallback                 (GLFWwindow *, int, int, int, int);

    static void                 charCallback                (GLFWwindow *, unsigned int);

    static void                 buttonCallback              (GLFWwindow*,int, int, int);

    static void                 scrollCallback              (GLFWwindow *, double, double yoffset);

    static void                 cursorPositionCallback      (GLFWwindow *, double xpos, double ypos);

    static void                 errorCallback               (int error, const char *description);

protected:
    GLFWwindow                 *m_pWindow;
    GLFWmonitor                *m_pMonitor;

    static Vector4              s_MousePosition;
    static Vector4              s_OldMousePosition;

    static int32_t              s_Width;
    static int32_t              s_Height;
    static bool                 s_Windowed;
    static bool                 s_vSync;
};

#endif // DESKTOPADAPTOR_H
