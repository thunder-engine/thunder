#ifndef DESKTOPAADAPTOR_H
#define DESKTOPAADAPTOR_H

#include "platformadaptor.h"

class Log;

class MobileAdaptor : public PlatformAdaptor {
public:
    MobileAdaptor               (Engine *engine);

    virtual ~MobileAdaptor      () {}

    bool                        init                        ();

    void                        update                      ();

    bool                        start                       ();

    void                        stop                        ();

    void                        destroy                     ();

    bool                        isValid                     ();

    string                      locationLocalDir            ();

    uint32_t                    screenWidth                 ();
    uint32_t                    screenHeight                ();

    string                      inputString                 ();
    void                        setKeyboardVisible          (bool visible);

    uint32_t                    touchCount                  ();
    uint32_t                    touchState                  (uint32_t index);
    Vector4                     touchPosition               (uint32_t index);

    uint32_t                    joystickCount               ();
    uint32_t                    joystickButtons             (uint32_t);
    Vector4                     joystickThumbs              (uint32_t);

public:
    static uint32_t             s_Buttons;

    static Vector2              s_Screen;

    static Vector4              s_Thumbs;

};

#endif // DESKTOPAADAPTOR_H
