#include "input.h"

#include "adapters/iplatformadaptor.h"

static IPlatformAdaptor *s_pPlatform = nullptr;

/*!
    \class Input
    \brief The interface to inpust system of Thunder Engine.
    \inmodule Engine

    Use this class to get information from the inpute devices, like mouse, keyboard, joystick and etc.

    \note All input data updates once per frame during Engine::update() method.

    \note Many mobile devices are capable of tracking multiple fingers touching the screen simultaneously.
*/
/*!
    Initialize the Input module.
    The \a platform adaptor is used to handle platform specific inputs.
    \note This method calls internally and must not be called manually.

    \internal
*/
void Input::init(IPlatformAdaptor *platform) {
    s_pPlatform = platform;
}
/*!
    Returns true in case of a key with \a code is pressed; otherwise returns false.
    Please refer to Input::KeyCode to see possible key codes.
*/
bool Input::isKey(KeyCode code) {
    return s_pPlatform->key(code);
}
/*!
    Returns true during the frame in case of a key with \a code is pressed; otherwise returns false.
    Please refer to Input::KeyCode to see possible key codes.
*/
bool Input::isKeyDown(KeyCode code) {
    return s_pPlatform->keyPressed(code);
}
/*!
    Returns true during the frame in case of a key with \a code is released; otherwise returns false.
    Please refer to Input::KeyCode to see possible key codes.
*/
bool Input::isKeyUp(KeyCode code) {
    return s_pPlatform->keyReleased(code);
}
/*!
    Returns the mouse position.
    The absolute position will be stored in x and y components.
    The normalized position will be stored in z and w components.
*/
Vector4 Input::mousePosition() {
    return s_pPlatform->mousePosition();
}
/*!
    Returns the mouse position delta.
    The absolute position will be stored in x and y components.
    The normalized position will be stored in z and w components.

    \note Delta value recalculated once per frame, calling this method multiple times in one frame will return the same result.
    \note The value will be Vector4(0.0f) if a mouse is not moved.
*/
Vector4 Input::mouseDelta() {
    return s_pPlatform->mouseDelta();
}
/*!
    Returns the states of mouse buttons.
    Please refer to Input::MouseButton to see possible buttons.
    Example code:
    \code
        if(Input::isMouseButton(Input::LEFT)) {
            ...
        }
    \endcode
*/
bool Input::isMouseButton(MouseButton button) {
    return s_pPlatform->mouseButton(button);
}
/*!
    Returns true in case of the \a button is pressed; otherwise returns false.
    Please refer to Input::MouseButton to see possible buttons.
*/
bool Input::isMouseButtonDown(MouseButton button) {
    return s_pPlatform->mousePressed(button);
}
/*!
    Returns true in case of the \a button is released; otherwise returns false.
    Please refer to Input::MouseButton to see possible buttons.
*/
bool Input::isMouseButtonUp(MouseButton button) {
    return s_pPlatform->mouseReleased(button);
}
/*!
    Moves the mouse cursor to the global screen position (\a x, \a y).
*/
void Input::setMousePosition(int32_t x, int32_t y) {
    s_pPlatform->setMousePosition(x, y);
}
/*!
    Returns the number of connected joysticks.
*/
uint32_t Input::joystickCount() {
    return s_pPlatform->joystickCount();
}
/*!
    Returns the states of buttons for joystick with \a index.
    Please refer to Input::JoystickButton to see possible buttons.
    \note This method returns a bit masked value. To retrieve the state of the required button please make bit comparison.

    Example code:
    \code
        if(Input::joystickButtons(0) & Input::START) {
            ...
        }
    \endcode
*/
uint32_t Input::joystickButtons(uint32_t index) {
    return s_pPlatform->joystickButtons(index);
}
/*!
    Returns the thumbs position of joystick with \a index.
    The components x and y will contain a value for the left thumbs.
    The components z and w will contain a value for the right thumbs.
*/
Vector4 Input::joystickThumbs(uint32_t index) {
    return s_pPlatform->joystickThumbs(index);
}
/*!
    Returns the value of pressure for the joystick triggers with \a index.
    The component x will contain a value for the left trigger and component y will contain value for the right trigger.
*/
Vector2 Input::joystickTriggers(uint32_t index) {
    return s_pPlatform->joystickTriggers(index);
}
/*!
    Returns the number of touches.
*/
uint32_t Input::touchCount() {
    return s_pPlatform->touchCount();
}
/*!
    Returns the state of touch with \a index.
    Please refer to Input::TouchState to see possible states.
*/
uint32_t Input::touchState(uint32_t index) {
    return s_pPlatform->touchState(index);
}
/*!
    Returns the position of touch with \a index.
    The absolute position will be stored in x and y components.
    The normalized position will be stored in z and w components.
*/
Vector4 Input::touchPosition(uint32_t index) {
    return s_pPlatform->touchPosition(index);
}
