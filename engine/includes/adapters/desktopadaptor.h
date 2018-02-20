#ifndef DESKTOPAADAPTOR_H
#define DESKTOPAADAPTOR_H

#include "iplatformadaptor.h"

class Log;
struct GLFWwindow;
struct GLFWmonitor;

class DesktopAdaptor : public IPlatformAdaptor {
public:
    virtual ~DesktopAdaptor     () {}

    bool                        init                        (Engine *engine);

    void                        update                      ();

    bool                        start                       ();

    void                        stop                        ();

    void                        destroy                     ();

    bool                        isValid                     ();

    bool                        key                         (Input::KeyCode code);

    Vector3                     mousePosition               ();

    Vector3                     mouseDelta                  ();

    uint8_t                     mouseButtons                ();

    uint32_t                    screenWidth                 ();

    uint32_t                    screenHeight                ();

    void                        setMousePosition            (const Vector3 &position);

    uint16_t                    joystickCount               ();

    uint16_t                    joystickButtons             (uint8_t index);

    Vector4                     joystickThumbs              (uint8_t index);

    Vector2                     joystickTriggers            (uint8_t index);

    void                       *pluginLoad                  (const char *name);

    bool                        pluginUnload                (void *plugin);

    void                       *pluginAddress               (void *plugin, const string &name);

protected:
    static void                 scrollCallback              (GLFWwindow *, double, double yoffset);

    static void                 cursorPositionCallback      (GLFWwindow *, double xpos, double ypos);

    static void                 errorCallback               (int error, const char *description);

protected:
    GLFWwindow                 *m_pWindow;

    GLFWmonitor                *m_pMonitor;

    uint8_t                     m_MouseButtons;

    static Vector3              m_MouseDelta;

    static Vector3              m_MousePosition;

};

#endif // DESKTOPAADAPTOR_H
