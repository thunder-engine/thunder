#ifndef AABSTRACTPLATFORMADAPTER_H
#define AABSTRACTPLATFORMADAPTER_H

#include <cstdint>
#include <string>

#include <amath.h>

#include "input.h"

class IPlatformAdaptor {
public:
    virtual ~IPlatformAdaptor           () {}

    virtual bool                        init                        (Engine *engine) = 0;

    virtual void                        update                      () = 0;

    virtual bool                        start                       () = 0;

    virtual void                        stop                        () = 0;

    virtual void                        destroy                     () = 0;

    virtual bool                        isValid                     () = 0;

    virtual bool                        key                         (Input::KeyCode code) = 0;

    virtual Vector3                     mousePosition               () = 0;

    virtual Vector3                     mouseDelta                  () = 0;

    virtual uint8_t                     mouseButtons                () = 0;

    virtual uint32_t                    screenWidth                 () = 0;

    virtual uint32_t                    screenHeight                () = 0;

    virtual void                        setMousePosition            (const Vector3 &position) = 0;

    virtual uint16_t                    joystickCount               () = 0;

    virtual uint16_t                    joystickButtons             (uint8_t index) = 0;

    virtual Vector4                     joystickThumbs              (uint8_t index) = 0;

    virtual Vector2                     joystickTriggers            (uint8_t index) = 0;

    virtual void                       *pluginLoad                  (const char *name) = 0;

    virtual bool                        pluginUnload                (void *plugin) = 0;

    virtual void                       *pluginAddress               (void *plugin, const string &name) = 0;

    virtual string                      locationLocalDir            () = 0;
};

#endif // AABSTRACTPLATFORMADAPTER_H
