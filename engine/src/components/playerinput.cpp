#include "components/playerinput.h"

#include "resources/controlscheme.h"

#include <input.h>

/*!
    \class PlayerInput
    \brief Allows to make an input bindings.
    \inmodule Components

    \note A PlayerInput controller simplifies the management of the player control scheme.
    It can be used to get the current state of actions from the assigned ControlScheme.
*/

PlayerInput::PlayerInput() :
        m_controlScheme(nullptr) {

}

PlayerInput::~PlayerInput() {
    if(m_controlScheme) {
        m_controlScheme->decRef();
    }
}

/*!
    \internal
*/
void PlayerInput::update() {
    for(auto &it : m_inputActions) {
        it.second.second = 0.0f;
        for(int i = 0; i < m_controlScheme->bindingsCount(it.second.first); i++) {
            int code = m_controlScheme->bindingCode(it.second.first, i);
            switch(code) {
                case Input::MOUSE_DELTA_X: it.second.second = Input::mouseDelta().x; break;
                case Input::MOUSE_DELTA_Y: it.second.second = Input::mouseDelta().y; break;
                default: {
                    if(Input::isKey((Input::KeyCode)code)) {
                        it.second.second += (m_controlScheme->bindingNegative(it.second.first, i) ? -1.0f : 1.0f);
                    }
                } break;
            }
        }
    }
}
/*!
    Returns the value of the virtual axis identified by \a name.
    The value will be in the range -1...1 for keyboard and joystick input devices.
*/
float PlayerInput::axis(const TString &name) {
    auto it = m_inputActions.find(name);
    if(it != m_inputActions.end()) {
        return it->second.second;
    }
    return 0.0f;
}
/*!
    Returns true in case of virtual button identified by \a name is pressed; otherwise returns false.
*/
bool PlayerInput::button(const TString &name) {
    auto it = m_inputActions.find(name);
    if(it != m_inputActions.end()) {
        float value = it->second.second;
        return value != 0.0f;
    }
    return false;
}
/*!
    Returns the current assigned control scheme.
*/
ControlScheme *PlayerInput::controlScheme() const {
    return m_controlScheme;
}
/*!
    \fn void PlayerInput::setControlScheme(ControlScheme *scheme)

    Assigns a new control \a scheme. All previous bindings and key states will be cleaned.
*/
void PlayerInput::setControlScheme(ControlScheme *scheme) {
    if(m_controlScheme != scheme) {
        if(m_controlScheme) {
            m_controlScheme->decRef();
        }

        m_controlScheme = scheme;

        m_inputActions.clear();

        if(m_controlScheme) {
            m_controlScheme->incRef();

            for(int a = 0; a < m_controlScheme->actionsCount(); a++) {
                m_inputActions[m_controlScheme->actionName(a)] = std::make_pair(a, 0.0f);
            }
        }
    }
}
