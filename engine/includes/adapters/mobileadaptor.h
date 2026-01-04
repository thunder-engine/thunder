#ifndef MOBILEADAPTOR_H
#define MOBILEADAPTOR_H

#include "platformadaptor.h"

class Log;

class MobileAdaptor : public PlatformAdaptor {
public:
    MobileAdaptor();

    virtual ~MobileAdaptor() {}

    bool init() override;

    void update() override;

    bool start() override;

    void destroy() override;

    bool isActive() const override;

    TString locationLocalDir() const override;

    uint32_t screenWidth() const override;
    uint32_t screenHeight() const override;

    bool key(Input::KeyCode code) const override;
    bool keyPressed(Input::KeyCode code) const override;
    bool keyReleased(Input::KeyCode code) const override;
    TString inputString() const override;
    void setKeyboardVisible(bool visible) override;

    Vector4 mousePosition() const override;
    Vector4 mouseDelta() const override;
    float mouseScrollDelta() const override;
    bool mouseButton(int button) const override;
    bool mousePressed(int button) const override;
    bool mouseReleased(int button) const override;

    void mouseLockCursor(bool lock) override;

    uint32_t touchCount() const override;
    uint32_t touchState(int index) const override;
    Vector4 touchPosition(int index) const override;

    uint32_t joystickCount() const override;
    uint32_t joystickButtons(int) const override;
    Vector4 joystickThumbs(int) const override;

public:
    static Vector4 s_thumbs;
    static Vector4 s_mousePosition;
    static Vector4 s_oldMousePosition;

    static int32_t s_width;
    static int32_t s_height;

    static float s_mouseScrollDelta;

    static bool s_mouseLocked;

    static bool s_appActive;
};

#endif // MOBILEADAPTOR_H
