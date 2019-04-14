#include "input.h"

#include "adapters/iplatformadaptor.h"

static IPlatformAdaptor *s_pPlatform = nullptr;

void Input::init(IPlatformAdaptor *platform) {
    s_pPlatform = platform;
}

bool Input::isKey(KeyCode code) {
    return s_pPlatform->key(code);
}

Vector4 Input::mousePosition() {
    return s_pPlatform->mousePosition();
}

Vector4 Input::mouseDelta() {
    return s_pPlatform->mouseDelta();
}

uint32_t Input::mouseButtons() {
    return s_pPlatform->mouseButtons();
}

void Input::setMousePosition(const Vector3 &position) {
    s_pPlatform->setMousePosition(position);
}

uint32_t Input::joystickCount() {
    return s_pPlatform->joystickCount();
}

uint32_t Input::joystickButtons(uint32_t index) {
    return s_pPlatform->joystickButtons(index);
}

Vector4 Input::joystickThumbs(uint32_t index) {
    return s_pPlatform->joystickThumbs(index);
}

Vector2 Input::joystickTriggers(uint32_t index) {
    return s_pPlatform->joystickTriggers(index);
}

uint32_t Input::touchCount() {
    return s_pPlatform->touchCount();
}

uint32_t Input::touchState(uint32_t index) {
    return s_pPlatform->touchState(index);
}

Vector4 Input::touchPosition(uint32_t index) {
    return s_pPlatform->touchPosition(index);
}
