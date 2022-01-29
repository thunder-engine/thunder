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

    string                      locationLocalDir            () const;

    uint32_t                    screenWidth                 () const;
    uint32_t                    screenHeight                () const;

    string                      inputString                 () const;
    void                        setKeyboardVisible          (bool visible);

    uint32_t                    touchCount                  () const;
    uint32_t                    touchState                  (uint32_t index) const;
    Vector4                     touchPosition               (uint32_t index) const;

    uint32_t                    joystickCount               () const;
    uint32_t                    joystickButtons             (uint32_t) const;
    Vector4                     joystickThumbs              (uint32_t) const;

public:
    static uint32_t             s_Buttons;

    static Vector2              s_Screen;

    static Vector4              s_Thumbs;

};

#endif // DESKTOPAADAPTOR_H
