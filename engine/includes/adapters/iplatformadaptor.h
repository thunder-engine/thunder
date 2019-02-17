#ifndef AABSTRACTPLATFORMADAPTER_H
#define AABSTRACTPLATFORMADAPTER_H

#include <cstdint>
#include <string>

#include <amath.h>

#include "input.h"

extern int thunderMain(Engine *engine);

class IPlatformAdaptor {
public:
    virtual ~IPlatformAdaptor           () {}

    virtual bool                        init                        () = 0;

    virtual void                        update                      () = 0;

    virtual bool                        start                       () = 0;

    virtual void                        stop                        () = 0;

    virtual void                        destroy                     () = 0;

    virtual bool                        isValid                     () = 0;

    virtual uint32_t                    screenWidth                 () = 0;

    virtual uint32_t                    screenHeight                () = 0;

    virtual bool                        key                         (Input::KeyCode code) { return false; }

    virtual Vector4                     mousePosition               () { return Vector4(); }

    virtual Vector4                     mouseDelta                  () { return Vector4(); }

    virtual uint8_t                     mouseButtons                () { return 0; }

    virtual void                        setMousePosition            (const Vector3 &position) { }

    virtual uint16_t                    joystickCount               () { return 0; }

    virtual uint16_t                    joystickButtons             (uint8_t index) { return 0; }

    virtual Vector4                     joystickThumbs              (uint8_t index) { return Vector4(); }

    virtual Vector2                     joystickTriggers            (uint8_t index) { return Vector2(); }

    virtual uint16_t                    touchCount                  () { return 0; }

    virtual uint16_t                    touchState                  (uint8_t index) { return 0; }

    virtual Vector4                     touchPosition               (uint8_t index) { return 0; }

    virtual void                       *pluginLoad                  (const char *name) { return nullptr; }

    virtual bool                        pluginUnload                (void *plugin) { return false; }

    virtual void                       *pluginAddress               (void *plugin, const string &name) { return nullptr; }

    virtual string                      locationLocalDir            () { return string(); }

};

#endif // AABSTRACTPLATFORMADAPTER_H
