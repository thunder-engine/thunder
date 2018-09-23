#ifndef DESKTOPAADAPTOR_H
#define DESKTOPAADAPTOR_H

#include "iplatformadaptor.h"

class Log;

class MobileAdaptor : public IPlatformAdaptor {
public:
    MobileAdaptor               (Engine *engine);

    virtual ~MobileAdaptor      () {}

    bool                        init                        ();

    void                        update                      ();

    bool                        start                       ();

    void                        stop                        ();

    void                        destroy                     ();

    bool                        isValid                     ();

    bool                        key                         (Input::KeyCode code);

    Vector4                     mousePosition               ();

    Vector4                     mouseDelta                  ();

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

    string                      locationLocalDir            ();

protected:


protected:
    uint8_t                     m_MouseButtons;

    static Vector4              m_MouseDelta;

    static Vector4              m_MousePosition;

};

#endif // DESKTOPAADAPTOR_H
