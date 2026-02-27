#ifndef INPUT_H
#define INPUT_H

#include <amath.h>

#include "engine.h"

class PlatformAdaptor;

class ENGINE_EXPORT Input {
public:
    enum KeyCode {
        KEY_UNKNOWN         = -1,
        KEY_SPACE           = 32,
        KEY_APOSTROPHE      = 39,
        KEY_COMMA           = 44,
        KEY_MINUS           = 45,
        KEY_PERIOD          = 46,
        KEY_SLASH           = 47,
        KEY_0               = 48,
        KEY_1               = 49,
        KEY_2               = 50,
        KEY_3               = 51,
        KEY_4               = 52,
        KEY_5               = 53,
        KEY_6               = 54,
        KEY_7               = 55,
        KEY_8               = 56,
        KEY_9               = 57,
        KEY_SEMICOLON       = 59,
        KEY_EQUAL           = 61,
        KEY_A               = 65,
        KEY_B               = 66,
        KEY_C               = 67,
        KEY_D               = 68,
        KEY_E               = 69,
        KEY_F               = 70,
        KEY_G               = 71,
        KEY_H               = 72,
        KEY_I               = 73,
        KEY_J               = 74,
        KEY_K               = 75,
        KEY_L               = 76,
        KEY_M               = 77,
        KEY_N               = 78,
        KEY_O               = 79,
        KEY_P               = 80,
        KEY_Q               = 81,
        KEY_R               = 82,
        KEY_S               = 83,
        KEY_T               = 84,
        KEY_U               = 85,
        KEY_V               = 86,
        KEY_W               = 87,
        KEY_X               = 88,
        KEY_Y               = 89,
        KEY_Z               = 90,
        KEY_LEFT_BRACKET    = 91,
        KEY_BACKSLASH       = 92,
        KEY_RIGHT_BRACKET   = 93,
        KEY_GRAVE_ACCENT    = 96,
        KEY_WORLD_1         = 16,
        KEY_WORLD_2         = 16,
        KEY_ESCAPE          = 256,
        KEY_ENTER           = 257,
        KEY_TAB             = 258,
        KEY_BACKSPACE       = 259,
        KEY_INSERT          = 260,
        KEY_DELETE          = 261,
        KEY_RIGHT           = 262,
        KEY_LEFT            = 263,
        KEY_DOWN            = 264,
        KEY_UP              = 265,
        KEY_PAGE_UP         = 266,
        KEY_PAGE_DOWN       = 267,
        KEY_HOME            = 268,
        KEY_END             = 269,
        KEY_CAPS_LOCK       = 280,
        KEY_SCROLL_LOCK     = 281,
        KEY_NUM_LOCK        = 282,
        KEY_PRINT_SCREEN    = 283,
        KEY_PAUSE           = 284,
        KEY_F1              = 290,
        KEY_F2              = 291,
        KEY_F3              = 292,
        KEY_F4              = 293,
        KEY_F5              = 294,
        KEY_F6              = 295,
        KEY_F7              = 296,
        KEY_F8              = 297,
        KEY_F9              = 298,
        KEY_F10             = 299,
        KEY_F11             = 300,
        KEY_F12             = 301,
        KEY_F13             = 302,
        KEY_F14             = 303,
        KEY_F15             = 304,
        KEY_F16             = 305,
        KEY_F17             = 306,
        KEY_F18             = 307,
        KEY_F19             = 308,
        KEY_F20             = 309,
        KEY_F21             = 310,
        KEY_F22             = 311,
        KEY_F23             = 312,
        KEY_F24             = 313,
        KEY_F25             = 314,
        KEY_KP_0            = 320,
        KEY_KP_1            = 321,
        KEY_KP_2            = 322,
        KEY_KP_3            = 323,
        KEY_KP_4            = 324,
        KEY_KP_5            = 325,
        KEY_KP_6            = 326,
        KEY_KP_7            = 327,
        KEY_KP_8            = 328,
        KEY_KP_9            = 329,
        KEY_KP_DECIMAL      = 330,
        KEY_KP_DIVIDE       = 331,
        KEY_KP_MULTIPLY     = 332,
        KEY_KP_SUBTRACT     = 333,
        KEY_KP_ADD          = 334,
        KEY_KP_ENTER        = 335,
        KEY_KP_EQUAL        = 336,
        KEY_LEFT_SHIFT      = 340,
        KEY_LEFT_CONTROL    = 341,
        KEY_LEFT_ALT        = 342,
        KEY_LEFT_SUPER      = 343,
        KEY_RIGHT_SHIFT     = 344,
        KEY_RIGHT_CONTROL   = 345,
        KEY_RIGHT_ALT       = 346,
        KEY_RIGHT_SUPER     = 347,
        KEY_MENU            = 348,

        MOUSE_LEFT          = 0x10000000,
        MOUSE_RIGHT,
        MOUSE_MIDDLE,
        MOUSE_BUTTON0,
        MOUSE_BUTTON1,
        MOUSE_BUTTON2,
        MOUSE_BUTTON3,
        MOUSE_BUTTON4,

        MOUSE_DELTA_X       = 0x10100000,
        MOUSE_DELTA_Y,

        JOYSTICK_UP_ARROW       = 0x20000001,
        JOYSTICK_DOWN_ARROW     = 0x20000002,
        JOYSTICK_LEFT_ARROW     = 0x20000004,
        JOYSTICK_RIGHT_ARROW    = 0x20000008,
        JOYSTICK_START          = 0x20000010,
        JOYSTICK_BACK           = 0x20000020,
        JOYSTICK_LEFT_THUMB     = 0x20000040,
        JOYSTICK_RIGHT_THUMB    = 0x20000080,
        JOYSTICK_LEFT_SHOULDER  = 0x20000100,
        JOYSTICK_RIGHT_SHOULDER = 0x20000200,
        JOYSTICK_A              = 0x20001000,
        JOYSTICK_B              = 0x20002000,
        JOYSTICK_X              = 0x20004000,
        JOYSTICK_Y              = 0x20008000,
    };

    enum TouchState {
        TOUCH_HOVER = 0,
        TOUCH_BEGAN,
        TOUCH_MOVED,
        TOUCH_ENDED,
        TOUCH_CANCELLED
    };

    enum CursorShape {
        CURSOR_ARROW = 0,
        CURSOR_CROSS,
        CURSOR_IBEAM,
        CURSOR_HAND,
        CURSOR_HORSIZE,
        CURSOR_VERSIZE,
        CURSOR_FDIAGSIZE,
        CURSOR_BDIAGSIZE,
        CURSOR_ALLSIZE,
    };

public:
    static void init(PlatformAdaptor *platform);

    static bool isKey(KeyCode code);
    static bool isKeyDown(KeyCode code);
    static bool isKeyUp(KeyCode code);

    static TString inputString();
    static void setKeyboardVisible(bool visible);

    static bool isMouseButton(int button);
    static bool isMouseButtonDown(int button);
    static bool isMouseButtonUp(int button);

    static Vector4 mousePosition();
    static Vector4 mouseDelta();
    static float mouseScrollDelta();

    static void mouseLockCursor(bool lock);
    static void mouseSetCursor(CursorShape shape);

    static uint32_t joystickCount();
    static uint32_t joystickButtons(uint32_t index);
    static Vector4 joystickThumbs(uint32_t index);
    static Vector2 joystickTriggers(uint32_t index);

    static uint32_t touchCount();
    static uint32_t touchState(uint32_t index);
    static Vector4 touchPosition(uint32_t index);

    static uint32_t getCode(const TString &name);

};

#endif // INPUT_H
