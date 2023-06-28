#ifndef DESKTOPAADAPTOR_H
#define DESKTOPAADAPTOR_H

#include "platformadaptor.h"

class Log;

class MobileAdaptor : public PlatformAdaptor {
public:
    MobileAdaptor();

    virtual ~MobileAdaptor() {}

    bool init() override;

    void update override

    bool start() override;

    void stop() override;

    void destroy() override;

    bool isValid() override;

    string locationLocalDir() const override;

    uint32_t screenWidth() const override;
    uint32_t screenHeight() const override;

    string inputString() const override;
    void setKeyboardVisible(bool visible) override;

    uint32_t touchCount() const override;
    uint32_t touchState(int index) const override;
    Vector4 touchPosition(int index) const override;

    uint32_t joystickCount() const override;
    uint32_t joystickButtons(int) const override;
    Vector4 joystickThumbs(int) const override;

public:
    static uint32_t s_Buttons;

    static Vector2 s_Screen;

    static Vector4 s_Thumbs;

};

#endif // DESKTOPAADAPTOR_H
