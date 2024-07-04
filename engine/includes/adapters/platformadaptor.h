#ifndef PLATFORMADAPTER_H
#define PLATFORMADAPTER_H

#include <cstdint>
#include <string>

#include <amath.h>

#include "input.h"

extern int thunderMain(Engine *engine);

class ENGINE_EXPORT PlatformAdaptor {
public:
    virtual ~PlatformAdaptor() {}

    virtual bool init() = 0;

    virtual void update() = 0;

    virtual bool start() = 0;

    virtual void stop() = 0;

    virtual void destroy() = 0;

    virtual bool isValid() = 0;

    virtual uint32_t screenWidth() const = 0;

    virtual uint32_t screenHeight() const = 0;

    virtual bool key(Input::KeyCode code) const;
    virtual bool keyPressed(Input::KeyCode code) const;
    virtual bool keyReleased(Input::KeyCode code) const;

    virtual std::string inputString() const = 0;
    virtual void setKeyboardVisible(bool visible);

    virtual Vector4 mousePosition() const;

    virtual Vector4 mouseDelta() const;
    virtual float mouseScrollDelta() const;

    virtual bool mouseButton(int code) const;
    virtual bool mousePressed(int code) const;
    virtual bool mouseReleased(int code) const;
    virtual void mouseLockCursor(bool lock);

    virtual uint32_t joystickCount() const;
    virtual uint32_t joystickButtons(int index) const;
    virtual Vector4 joystickThumbs(int index) const;
    virtual Vector2 joystickTriggers(int index) const;

    virtual uint32_t touchCount() const;
    virtual uint32_t touchState(int index) const;
    virtual Vector4 touchPosition(int index) const;

    virtual void *pluginLoad(const char *name);

    virtual bool pluginUnload(void *plugin);

    virtual void *pluginAddress(void *plugin, const std::string &name);

    virtual std::string locationLocalDir() const;

    virtual void syncConfiguration(VariantMap &map) const;

};

#endif // PLATFORMADAPTER_H
