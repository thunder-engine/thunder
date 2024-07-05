#ifndef PLAYERINPUT_H
#define PLAYERINPUT_H

#include <nativebehaviour.h>

#include <controlscheme.h>

class ENGINE_EXPORT PlayerInput : public NativeBehaviour {
    A_REGISTER(PlayerInput, NativeBehaviour, Components)

    A_PROPERTIES(
        A_PROPERTY(ControlScheme *, controlScheme, PlayerInput::controlScheme, PlayerInput::setControlScheme)
    )

public:
    PlayerInput();
    ~PlayerInput();

    float axis(const std::string &name);
    bool button(const std::string &name);

    ControlScheme *controlScheme() const;
    void setControlScheme(ControlScheme *scheme);

private:
    void update() override;

private:
    ControlScheme *m_controlScheme;

    std::unordered_map<std::string, std::pair<int, float>> m_inputActions;

};

#endif // PLAYERINPUT_H
