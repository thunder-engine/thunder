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

    virtual uint32_t                    mouseButtons                () { return 0; }

    virtual void                        setMousePosition            (int32_t x, int32_t y) { }

    virtual uint32_t                    joystickCount               () { return 0; }

    virtual uint32_t                    joystickButtons             (uint32_t index) { return 0; }

    virtual Vector4                     joystickThumbs              (uint32_t index) { return Vector4(); }

    virtual Vector2                     joystickTriggers            (uint32_t index) { return Vector2(); }

    virtual uint32_t                    touchCount                  () { return 0; }

    virtual uint32_t                    touchState                  (uint32_t index) { return 0; }

    virtual Vector4                     touchPosition               (uint32_t index) { return 0; }

    virtual void                       *pluginLoad                  (const char *name) { return nullptr; }

    virtual bool                        pluginUnload                (void *plugin) { return false; }

    virtual void                       *pluginAddress               (void *plugin, const string &name) { return nullptr; }

    virtual string                      locationLocalDir            () { return string(); }

    virtual void                        syncConfiguration           (VariantMap &map) const { }

};

#endif // AABSTRACTPLATFORMADAPTER_H
