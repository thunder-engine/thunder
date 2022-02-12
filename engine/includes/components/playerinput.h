#ifndef PLAYERINPUT_H
#define PLAYERINPUT_H

#include <nativebehaviour.h>

class ControlScheme;
class PlayerInputPrivate;

class ENGINE_EXPORT PlayerInput : public NativeBehaviour {
    A_REGISTER(PlayerInput, NativeBehaviour, Components)

    A_PROPERTIES(
        A_PROPERTY(ControlScheme *, Control_Scheme, PlayerInput::controlScheme, PlayerInput::setControlScheme)
    )

public:
    PlayerInput();

    float axis(const string &name);
    bool button(const string &name);

    ControlScheme *controlScheme() const;
    void setControlScheme(ControlScheme *scheme);

private:
    void update() override;

    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

private:
    PlayerInputPrivate *p_ptr;

};

#endif // PLAYERINPUT_H
