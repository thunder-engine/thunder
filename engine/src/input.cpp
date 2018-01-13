#include <log.h>

#include "input.h"

#include "adapters/iplatformadaptor.h"

/*!
    Constructor of Input class.
    @param[in]  pLog        The pointer to Log object for working with logging system.
*/
Input::Input() :
        m_pPlatform(nullptr) {

}

Input::~Input() {

}

void Input::init(IPlatformAdaptor *platform) {
    m_pPlatform = platform;
}

bool Input::key(KeyCode code) {
    return m_pPlatform->key(code);
}

Vector3 Input::mousePosition() {
    return m_pPlatform->mousePosition();
}

Vector3 Input::mouseDelta() {
    return m_pPlatform->mouseDelta();
}

uint8_t Input::mouseButtons() {
    return m_pPlatform->mouseButtons();
}

void Input::setMousePosition(const Vector3 &position) {
    m_pPlatform->setMousePosition(position);
}

uint16_t Input::joystickCount() {
    return m_pPlatform->joystickCount();
}

uint16_t Input::joystickButtons(uint8_t index) {
    return m_pPlatform->joystickButtons(index);
}

Vector4 Input::joystickThumbs(uint8_t index) {
    return m_pPlatform->joystickThumbs(index);
}

Vector2 Input::joystickTriggers(uint8_t index) {
    return m_pPlatform->joystickTriggers(index);
}
