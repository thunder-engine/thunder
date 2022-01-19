#include "components/playerinput.h"

#include "resources/controlscheme.h"

#include <input.h>

#define SCHEME "Scheme"

class PlayerInputPrivate {
public:
    PlayerInputPrivate() :
        m_controlScheme(nullptr){

    }

    struct Action {
        int index;
        float currentValue;
    };

    ControlScheme *m_controlScheme;

    unordered_map<string, Action> m_inputActions;
};

/*!
    \class PlayerInput
    \brief Allows to make an input bindings.
    \inmodule Engine

    \note A PlayerInput controller simplifies the management of the player control scheme.
    It can be used to get the current state of actions from the assigned ControlScheme.
*/

PlayerInput::PlayerInput() :
    p_ptr(new PlayerInputPrivate) {

}
/*!
    \internal
*/
void PlayerInput::update() {
    for(auto &it : p_ptr->m_inputActions) {
        it.second.currentValue = 0.0f;
        for(int i = 0; i < p_ptr->m_controlScheme->bindingsCount(it.second.index); i++) {
            int code = p_ptr->m_controlScheme->bindingCode(it.second.index, i);
            switch(code) {
                case Input::MOUSE_DELTA_X: it.second.currentValue = Input::mouseDelta().x; break;
                case Input::MOUSE_DELTA_Y: it.second.currentValue = Input::mouseDelta().y; break;
                default: {
                    if(Input::isKey((Input::KeyCode)code)) {
                        it.second.currentValue += p_ptr->m_controlScheme->bindingNegative(it.second.index, i) ? -1.0f : 1.0f;
                    }
                } break;
            }
        }
    }
}
/*!
    Returns the value of the virtual axis identified by name.
    The value will be in the range -1...1 for keyboard and joystick input devices.
*/
float PlayerInput::axis(const string &name) {
    auto it = p_ptr->m_inputActions.find(name);
    if(it != p_ptr->m_inputActions.end()) {
        return it->second.currentValue;
    }
    return 0.0f;
}
/*!
    Returns true in case of virtual button identified by \a name is pressed; otherwise returns false.
*/
bool PlayerInput::button(const string &name) {
    auto it = p_ptr->m_inputActions.find(name);
    if(it != p_ptr->m_inputActions.end()) {
        return it->second.currentValue != 0.0f;
    }
    return false;
}
/*!
    Returns the current assigned control scheme.
*/
ControlScheme *PlayerInput::controlScheme() const {
    return p_ptr->m_controlScheme;
}
/*!
    Assigns a new control \a scheme. All previous bindings and key states will be cleaned.
*/
void PlayerInput::setControlScheme(ControlScheme *scheme) {
    p_ptr->m_controlScheme = scheme;

    p_ptr->m_inputActions.clear();

    if(p_ptr->m_controlScheme) {
        for(int a = 0; a < p_ptr->m_controlScheme->actionsCount(); a++) {
            PlayerInputPrivate::Action action;

            action.index = a;
            action.currentValue = 0.0f;

            p_ptr->m_inputActions[p_ptr->m_controlScheme->actionName(a)] = action;
        }
    }
}
/*!
    \internal
*/
void PlayerInput::loadUserData(const VariantMap &data) {
    NativeBehaviour::loadUserData(data);
    {
        auto it = data.find(SCHEME);
        if(it != data.end()) {
            setControlScheme(Engine::loadResource<ControlScheme>((*it).second.toString()));
        }
    }
}
/*!
    \internal
*/
VariantMap PlayerInput::saveUserData() const {
    VariantMap result = NativeBehaviour::saveUserData();
    {
        string ref = Engine::reference(controlScheme());
        if(!ref.empty()) {
            result[SCHEME] = ref;
        }
    }
    return result;
}
