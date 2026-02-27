#include "input.h"

#include "adapters/platformadaptor.h"

static PlatformAdaptor *s_pPlatform = nullptr;

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
void Input::init(PlatformAdaptor *platform) {
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
    Returns characters entered since the last frame.
*/
TString Input::inputString() {
    return s_pPlatform->inputString();
}
/*!
    Sets virtual keyboard \a visible.
    \note Does nothing for the desktop platforms.
*/
void Input::setKeyboardVisible(bool visible) {
    s_pPlatform->setKeyboardVisible(visible);
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
    Returns the mouse wheel scrolling delta.

    \note Delta value recalculated once per frame, calling this method multiple times in one frame will return the same result.
    \note The value will be 0.0f if a mouse wheel is not rotated.
*/
float Input::mouseScrollDelta() {
    return s_pPlatform->mouseScrollDelta();
}
/*!
    Tries to \a lock mouse cursor.
*/
void Input::mouseLockCursor(bool lock) {
    s_pPlatform->mouseLockCursor(lock);
}
/*!
    Sets the cursor \a shape.
*/
void Input::mouseSetCursor(CursorShape shape) {
    s_pPlatform->mouseSetCursor(shape);
}
/*!
    Returns the state of mouse \a button.
    Example code:
    \code
        if(Input::isMouseButton(0)) {
            aInfo() << "Left button pressed";
        }
        if(Input::isMouseButton(1)) {
            aInfo() << "Right button pressed";
        }
        if(Input::isMouseButton(3)) {
            aInfo() << "Middle button pressed";
        }
    \endcode
*/
bool Input::isMouseButton(int button) {
    return s_pPlatform->mouseButton(button);
}
/*!
    Returns true in case of the \a button is pressed; otherwise returns false.
*/
bool Input::isMouseButtonDown(int button) {
    return s_pPlatform->mousePressed(button);
}
/*!
    Returns true in case of the \a button is released; otherwise returns false.
*/
bool Input::isMouseButtonUp(int button) {
    return s_pPlatform->mouseReleased(button);
}
/*!
    Returns the number of connected joysticks.
*/
uint32_t Input::joystickCount() {
    return s_pPlatform->joystickCount();
}
/*!
    Returns the states of buttons for joystick with \a index.
    Please refer to Input::KeyCode to see possible buttons.
    \note This method returns a bit masked value. To retrieve the state of the required button please make bit comparison.

    Example code:
    \code
        if(Input::joystickButtons(0) & Input::JOYSTICK_) {
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
/*!
    Converts a key \a name to code.
*/
uint32_t Input::getCode(const TString &name) {
    const static std::map<TString, uint32_t> keys {
        { "space", KEY_SPACE },
        { "'", KEY_APOSTROPHE },
        { ",", KEY_COMMA },
        { "-", KEY_MINUS },
        { ".", KEY_PERIOD },
        { "\\", KEY_SLASH },
        { "0", KEY_0 },
        { "1", KEY_1 },
        { "2", KEY_2 },
        { "3", KEY_3 },
        { "4", KEY_4 },
        { "5", KEY_5 },
        { "6", KEY_6 },
        { "7", KEY_7 },
        { "8", KEY_8 },
        { "9", KEY_9 },
        { ";", KEY_SEMICOLON },
        { "=", KEY_EQUAL },
        { "a", KEY_A },
        { "b", KEY_B },
        { "c", KEY_C },
        { "d", KEY_D },
        { "e", KEY_E },
        { "f", KEY_F },
        { "g", KEY_G },
        { "h", KEY_H },
        { "i", KEY_I },
        { "j", KEY_J },
        { "k", KEY_K },
        { "l", KEY_L },
        { "m", KEY_M },
        { "n", KEY_N },
        { "o", KEY_O },
        { "p", KEY_P },
        { "q", KEY_Q },
        { "r", KEY_R },
        { "s", KEY_S },
        { "t", KEY_T },
        { "u", KEY_U },
        { "v", KEY_V },
        { "w", KEY_W },
        { "x", KEY_X },
        { "y", KEY_Y },
        { "z", KEY_Z },
        { "(", KEY_LEFT_BRACKET },
        { "/", KEY_BACKSLASH },
        { ")", KEY_RIGHT_BRACKET },
        { "`", KEY_GRAVE_ACCENT },
     // KEY_WORLD_1         = 16,
     // KEY_WORLD_2         = 16,
        { "esc", KEY_ESCAPE },
        { "enter", KEY_ENTER },
        { "tab", KEY_TAB },
        { "backspace", KEY_BACKSPACE },
        { "insert", KEY_INSERT },
        { "delete", KEY_DELETE },
        { "right", KEY_RIGHT },
        { "left", KEY_LEFT },
        { "down", KEY_DOWN },
        { "up", KEY_UP },
        { "page up", KEY_PAGE_UP },
        { "page down", KEY_PAGE_DOWN },
        { "home", KEY_HOME },
        { "end", KEY_END },
        { "caps lock", KEY_CAPS_LOCK },
        { "scroll lock", KEY_SCROLL_LOCK },
        { "num lock", KEY_NUM_LOCK },
        { "print screen", KEY_PRINT_SCREEN },
        { "pause", KEY_PAUSE },
        { "f1", KEY_F1 },
        { "f2", KEY_F2 },
        { "f3", KEY_F3 },
        { "f4", KEY_F4 },
        { "f5", KEY_F5 },
        { "f6", KEY_F6 },
        { "f7", KEY_F7 },
        { "f8", KEY_F8 },
        { "f9", KEY_F9 },
        { "f10", KEY_F10 },
        { "f11", KEY_F11 },
        { "f12", KEY_F12 },
        { "f13", KEY_F13 },
        { "f14", KEY_F14 },
        { "f15", KEY_F15 },
        { "f16", KEY_F16 },
        { "f17", KEY_F17 },
        { "f18", KEY_F18 },
        { "f19", KEY_F19 },
        { "f20", KEY_F20 },
        { "f21", KEY_F21 },
        { "f22", KEY_F22 },
        { "f23", KEY_F23 },
        { "f24", KEY_F24 },
        { "f25", KEY_F25 },
        { "num 0", KEY_KP_0 },
        { "num 1", KEY_KP_1 },
        { "num 2", KEY_KP_2 },
        { "num 3", KEY_KP_3 },
        { "num 4", KEY_KP_4 },
        { "num 5", KEY_KP_5 },
        { "num 6", KEY_KP_6 },
        { "num 7", KEY_KP_7 },
        { "num 8", KEY_KP_8 },
        { "num 9", KEY_KP_9 },
        { "num .", KEY_KP_DECIMAL },
        { "num /", KEY_KP_DIVIDE },
        { "num *", KEY_KP_MULTIPLY },
        { "num -", KEY_KP_SUBTRACT },
        { "num +", KEY_KP_ADD },
        { "num enter", KEY_KP_ENTER },
        { "num =", KEY_KP_EQUAL },
        { "left shift", KEY_LEFT_SHIFT },
        { "left ctrl", KEY_LEFT_CONTROL },
        { "left alt", KEY_LEFT_ALT },
        { "left super", KEY_LEFT_SUPER },
        { "right shift", KEY_RIGHT_SHIFT },
        { "right ctrl", KEY_RIGHT_CONTROL },
        { "right alt", KEY_RIGHT_ALT },
        { "right super", KEY_RIGHT_SUPER },
        { "menu", KEY_MENU },

        { "moude left", MOUSE_LEFT },
        { "moude right", MOUSE_RIGHT },
        { "moude middle", MOUSE_MIDDLE },
        { "moude 3", MOUSE_BUTTON0 },
        { "moude 4", MOUSE_BUTTON1 },
        { "moude 5", MOUSE_BUTTON2 },
        { "moude 6", MOUSE_BUTTON3 },
        { "moude 7", MOUSE_BUTTON4 },

        { "joystick up arrow", JOYSTICK_UP_ARROW },
        { "joystick down arrow", JOYSTICK_DOWN_ARROW },
        { "joystick left arrow", JOYSTICK_LEFT_ARROW },
        { "joystick right arrow", JOYSTICK_RIGHT_ARROW },
        { "joystick start", JOYSTICK_START },
        { "joystick back", JOYSTICK_BACK },
        { "joystick left thumb", JOYSTICK_LEFT_THUMB },
        { "joystick right thumb", JOYSTICK_RIGHT_THUMB },
        { "joystick left shoulder", JOYSTICK_LEFT_SHOULDER },
        { "joystick right shoulder", JOYSTICK_RIGHT_SHOULDER },
        { "joystick a", JOYSTICK_A },
        { "joystick b", JOYSTICK_B },
        { "joystick x", JOYSTICK_X },
        { "joystick y", JOYSTICK_Y },
    };
    auto it = keys.find(name);
    if(it != keys.end()) {
        return it->second;
    }
    return KEY_UNKNOWN;
}
