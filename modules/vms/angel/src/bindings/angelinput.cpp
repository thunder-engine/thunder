#include "bindings/angelbindings.h"

#include <angelscript.h>
#include <autowrapper/aswrappedcall.h>

#include <input.h>

namespace {
    const char *gMouseButton("MouseButton");
    const char *gCursorShape("CursorShape");
    const char *gJoystickButton("JoystickButton");
    const char *gTouchState("TouchState");
    const char *gKeyCode("KeyCode");
}

void registerInput(asIScriptEngine *engine, bool generic) {
    engine->SetDefaultNamespace("Input");

    engine->RegisterEnum(gKeyCode);
    engine->RegisterEnumValue(gKeyCode, "KEY_UNKNOWN",           Input::KEY_UNKNOWN);
    engine->RegisterEnumValue(gKeyCode, "KEY_SPACE",             Input::KEY_SPACE);
    engine->RegisterEnumValue(gKeyCode, "KEY_APOSTROPHE",        Input::KEY_APOSTROPHE);
    engine->RegisterEnumValue(gKeyCode, "KEY_COMMA",             Input::KEY_COMMA);
    engine->RegisterEnumValue(gKeyCode, "KEY_MINUS",             Input::KEY_MINUS);
    engine->RegisterEnumValue(gKeyCode, "KEY_PERIOD",            Input::KEY_PERIOD);
    engine->RegisterEnumValue(gKeyCode, "KEY_SLASH",             Input::KEY_SLASH);
    engine->RegisterEnumValue(gKeyCode, "KEY_0",                 Input::KEY_0);
    engine->RegisterEnumValue(gKeyCode, "KEY_1",                 Input::KEY_1);
    engine->RegisterEnumValue(gKeyCode, "KEY_2",                 Input::KEY_2);
    engine->RegisterEnumValue(gKeyCode, "KEY_3",                 Input::KEY_3);
    engine->RegisterEnumValue(gKeyCode, "KEY_4",                 Input::KEY_4);
    engine->RegisterEnumValue(gKeyCode, "KEY_5",                 Input::KEY_5);
    engine->RegisterEnumValue(gKeyCode, "KEY_6",                 Input::KEY_6);
    engine->RegisterEnumValue(gKeyCode, "KEY_7",                 Input::KEY_7);
    engine->RegisterEnumValue(gKeyCode, "KEY_8",                 Input::KEY_8);
    engine->RegisterEnumValue(gKeyCode, "KEY_9",                 Input::KEY_9);
    engine->RegisterEnumValue(gKeyCode, "KEY_SEMICOLON",         Input::KEY_SEMICOLON);
    engine->RegisterEnumValue(gKeyCode, "KEY_EQUAL",             Input::KEY_EQUAL);
    engine->RegisterEnumValue(gKeyCode, "KEY_A",                 Input::KEY_A);
    engine->RegisterEnumValue(gKeyCode, "KEY_B",                 Input::KEY_B);
    engine->RegisterEnumValue(gKeyCode, "KEY_C",                 Input::KEY_C);
    engine->RegisterEnumValue(gKeyCode, "KEY_D",                 Input::KEY_D);
    engine->RegisterEnumValue(gKeyCode, "KEY_E",                 Input::KEY_E);
    engine->RegisterEnumValue(gKeyCode, "KEY_F",                 Input::KEY_F);
    engine->RegisterEnumValue(gKeyCode, "KEY_G",                 Input::KEY_G);
    engine->RegisterEnumValue(gKeyCode, "KEY_H",                 Input::KEY_H);
    engine->RegisterEnumValue(gKeyCode, "KEY_I",                 Input::KEY_I);
    engine->RegisterEnumValue(gKeyCode, "KEY_J",                 Input::KEY_J);
    engine->RegisterEnumValue(gKeyCode, "KEY_K",                 Input::KEY_K);
    engine->RegisterEnumValue(gKeyCode, "KEY_L",                 Input::KEY_L);
    engine->RegisterEnumValue(gKeyCode, "KEY_M",                 Input::KEY_M);
    engine->RegisterEnumValue(gKeyCode, "KEY_N",                 Input::KEY_N);
    engine->RegisterEnumValue(gKeyCode, "KEY_O",                 Input::KEY_O);
    engine->RegisterEnumValue(gKeyCode, "KEY_P",                 Input::KEY_P);
    engine->RegisterEnumValue(gKeyCode, "KEY_Q",                 Input::KEY_Q);
    engine->RegisterEnumValue(gKeyCode, "KEY_R",                 Input::KEY_R);
    engine->RegisterEnumValue(gKeyCode, "KEY_S",                 Input::KEY_S);
    engine->RegisterEnumValue(gKeyCode, "KEY_T",                 Input::KEY_T);
    engine->RegisterEnumValue(gKeyCode, "KEY_U",                 Input::KEY_U);
    engine->RegisterEnumValue(gKeyCode, "KEY_V",                 Input::KEY_V);
    engine->RegisterEnumValue(gKeyCode, "KEY_W",                 Input::KEY_W);
    engine->RegisterEnumValue(gKeyCode, "KEY_X",                 Input::KEY_X);
    engine->RegisterEnumValue(gKeyCode, "KEY_Y",                 Input::KEY_Y);
    engine->RegisterEnumValue(gKeyCode, "KEY_Z",                 Input::KEY_Z);
    engine->RegisterEnumValue(gKeyCode, "KEY_LEFT_BRACKET",      Input::KEY_LEFT_BRACKET);
    engine->RegisterEnumValue(gKeyCode, "KEY_BACKSLASH",         Input::KEY_BACKSLASH);
    engine->RegisterEnumValue(gKeyCode, "KEY_RIGHT_BRACKET",     Input::KEY_RIGHT_BRACKET);
    engine->RegisterEnumValue(gKeyCode, "KEY_GRAVE_ACCENT",      Input::KEY_GRAVE_ACCENT);
    engine->RegisterEnumValue(gKeyCode, "KEY_WORLD_1",           Input::KEY_WORLD_1);
    engine->RegisterEnumValue(gKeyCode, "KEY_WORLD_2",           Input::KEY_WORLD_2);
    engine->RegisterEnumValue(gKeyCode, "KEY_ESCAPE",            Input::KEY_ESCAPE);
    engine->RegisterEnumValue(gKeyCode, "KEY_ENTER",             Input::KEY_ENTER);
    engine->RegisterEnumValue(gKeyCode, "KEY_TAB",               Input::KEY_TAB);
    engine->RegisterEnumValue(gKeyCode, "KEY_BACKSPACE",         Input::KEY_BACKSPACE);
    engine->RegisterEnumValue(gKeyCode, "KEY_INSERT",            Input::KEY_INSERT);
    engine->RegisterEnumValue(gKeyCode, "KEY_DELETE",            Input::KEY_DELETE);
    engine->RegisterEnumValue(gKeyCode, "KEY_RIGHT",             Input::KEY_RIGHT);
    engine->RegisterEnumValue(gKeyCode, "KEY_LEFT",              Input::KEY_LEFT);
    engine->RegisterEnumValue(gKeyCode, "KEY_DOWN",              Input::KEY_DOWN);
    engine->RegisterEnumValue(gKeyCode, "KEY_UP",                Input::KEY_UP);
    engine->RegisterEnumValue(gKeyCode, "KEY_PAGE_UP",           Input::KEY_PAGE_UP);
    engine->RegisterEnumValue(gKeyCode, "KEY_PAGE_DOWN",         Input::KEY_PAGE_DOWN);
    engine->RegisterEnumValue(gKeyCode, "KEY_HOME",              Input::KEY_HOME);
    engine->RegisterEnumValue(gKeyCode, "KEY_END",               Input::KEY_END);
    engine->RegisterEnumValue(gKeyCode, "KEY_CAPS_LOCK",         Input::KEY_CAPS_LOCK);
    engine->RegisterEnumValue(gKeyCode, "KEY_SCROLL_LOCK",       Input::KEY_SCROLL_LOCK);
    engine->RegisterEnumValue(gKeyCode, "KEY_NUM_LOCK",          Input::KEY_NUM_LOCK);
    engine->RegisterEnumValue(gKeyCode, "KEY_PRINT_SCREEN",      Input::KEY_PRINT_SCREEN);
    engine->RegisterEnumValue(gKeyCode, "KEY_PAUSE",             Input::KEY_PAUSE);
    engine->RegisterEnumValue(gKeyCode, "KEY_F1",                Input::KEY_F1);
    engine->RegisterEnumValue(gKeyCode, "KEY_F2",                Input::KEY_F2);
    engine->RegisterEnumValue(gKeyCode, "KEY_F3",                Input::KEY_F3);
    engine->RegisterEnumValue(gKeyCode, "KEY_F4",                Input::KEY_F4);
    engine->RegisterEnumValue(gKeyCode, "KEY_F5",                Input::KEY_F5);
    engine->RegisterEnumValue(gKeyCode, "KEY_F6",                Input::KEY_F6);
    engine->RegisterEnumValue(gKeyCode, "KEY_F7",                Input::KEY_F7);
    engine->RegisterEnumValue(gKeyCode, "KEY_F8",                Input::KEY_F8);
    engine->RegisterEnumValue(gKeyCode, "KEY_F9",                Input::KEY_F9);
    engine->RegisterEnumValue(gKeyCode, "KEY_F10",               Input::KEY_F10);
    engine->RegisterEnumValue(gKeyCode, "KEY_F11",               Input::KEY_F11);
    engine->RegisterEnumValue(gKeyCode, "KEY_F12",               Input::KEY_F12);
    engine->RegisterEnumValue(gKeyCode, "KEY_F13",               Input::KEY_F13);
    engine->RegisterEnumValue(gKeyCode, "KEY_F14",               Input::KEY_F14);
    engine->RegisterEnumValue(gKeyCode, "KEY_F15",               Input::KEY_F15);
    engine->RegisterEnumValue(gKeyCode, "KEY_F16",               Input::KEY_F16);
    engine->RegisterEnumValue(gKeyCode, "KEY_F17",               Input::KEY_F17);
    engine->RegisterEnumValue(gKeyCode, "KEY_F18",               Input::KEY_F18);
    engine->RegisterEnumValue(gKeyCode, "KEY_F19",               Input::KEY_F19);
    engine->RegisterEnumValue(gKeyCode, "KEY_F20",               Input::KEY_F20);
    engine->RegisterEnumValue(gKeyCode, "KEY_F21",               Input::KEY_F21);
    engine->RegisterEnumValue(gKeyCode, "KEY_F22",               Input::KEY_F22);
    engine->RegisterEnumValue(gKeyCode, "KEY_F23",               Input::KEY_F23);
    engine->RegisterEnumValue(gKeyCode, "KEY_F24",               Input::KEY_F24);
    engine->RegisterEnumValue(gKeyCode, "KEY_F25",               Input::KEY_F25);
    engine->RegisterEnumValue(gKeyCode, "KEY_KP_0",              Input::KEY_KP_0);
    engine->RegisterEnumValue(gKeyCode, "KEY_KP_1",              Input::KEY_KP_1);
    engine->RegisterEnumValue(gKeyCode, "KEY_KP_2",              Input::KEY_KP_2);
    engine->RegisterEnumValue(gKeyCode, "KEY_KP_3",              Input::KEY_KP_3);
    engine->RegisterEnumValue(gKeyCode, "KEY_KP_4",              Input::KEY_KP_4);
    engine->RegisterEnumValue(gKeyCode, "KEY_KP_5",              Input::KEY_KP_5);
    engine->RegisterEnumValue(gKeyCode, "KEY_KP_6",              Input::KEY_KP_6);
    engine->RegisterEnumValue(gKeyCode, "KEY_KP_7",              Input::KEY_KP_7);
    engine->RegisterEnumValue(gKeyCode, "KEY_KP_8",              Input::KEY_KP_8);
    engine->RegisterEnumValue(gKeyCode, "KEY_KP_9",              Input::KEY_KP_9);
    engine->RegisterEnumValue(gKeyCode, "KEY_KP_DECIMAL",        Input::KEY_KP_DECIMAL);
    engine->RegisterEnumValue(gKeyCode, "KEY_KP_DIVIDE",         Input::KEY_KP_DIVIDE);
    engine->RegisterEnumValue(gKeyCode, "KEY_KP_MULTIPLY",       Input::KEY_KP_MULTIPLY);
    engine->RegisterEnumValue(gKeyCode, "KEY_KP_SUBTRACT",       Input::KEY_KP_SUBTRACT);
    engine->RegisterEnumValue(gKeyCode, "KEY_KP_ADD",            Input::KEY_KP_ADD);
    engine->RegisterEnumValue(gKeyCode, "KEY_KP_ENTER",          Input::KEY_KP_ENTER);
    engine->RegisterEnumValue(gKeyCode, "KEY_KP_EQUAL",          Input::KEY_KP_EQUAL);
    engine->RegisterEnumValue(gKeyCode, "KEY_LEFT_SHIFT",        Input::KEY_LEFT_SHIFT);
    engine->RegisterEnumValue(gKeyCode, "KEY_LEFT_CONTROL",      Input::KEY_LEFT_CONTROL);
    engine->RegisterEnumValue(gKeyCode, "KEY_LEFT_ALT",          Input::KEY_LEFT_ALT);
    engine->RegisterEnumValue(gKeyCode, "KEY_LEFT_SUPER",        Input::KEY_LEFT_SUPER);
    engine->RegisterEnumValue(gKeyCode, "KEY_RIGHT_SHIFT",       Input::KEY_RIGHT_SHIFT);
    engine->RegisterEnumValue(gKeyCode, "KEY_RIGHT_CONTROL",     Input::KEY_RIGHT_CONTROL);
    engine->RegisterEnumValue(gKeyCode, "KEY_RIGHT_ALT",         Input::KEY_RIGHT_ALT);
    engine->RegisterEnumValue(gKeyCode, "KEY_RIGHT_SUPER",       Input::KEY_RIGHT_SUPER);
    engine->RegisterEnumValue(gKeyCode, "KEY_MENU",              Input::KEY_MENU);

    engine->RegisterEnum(gMouseButton);
    engine->RegisterEnumValue(gMouseButton, "MOUSE_LEFT",      Input::MOUSE_LEFT);
    engine->RegisterEnumValue(gMouseButton, "MOUSE_RIGHT",     Input::MOUSE_RIGHT);
    engine->RegisterEnumValue(gMouseButton, "MOUSE_MIDDLE",    Input::MOUSE_MIDDLE);
    engine->RegisterEnumValue(gMouseButton, "MOUSE_BUTTON0",   Input::MOUSE_BUTTON0);
    engine->RegisterEnumValue(gMouseButton, "MOUSE_BUTTON1",   Input::MOUSE_BUTTON1);
    engine->RegisterEnumValue(gMouseButton, "MOUSE_BUTTON2",   Input::MOUSE_BUTTON2);
    engine->RegisterEnumValue(gMouseButton, "MOUSE_BUTTON3",   Input::MOUSE_BUTTON3);
    engine->RegisterEnumValue(gMouseButton, "MOUSE_BUTTON4",   Input::MOUSE_BUTTON4);

    engine->RegisterEnum(gJoystickButton);
    engine->RegisterEnumValue(gJoystickButton, "JOYSTICK_UP_ARROW",       Input::JOYSTICK_UP_ARROW);
    engine->RegisterEnumValue(gJoystickButton, "JOYSTICK_DOWN_ARROW",     Input::JOYSTICK_DOWN_ARROW);
    engine->RegisterEnumValue(gJoystickButton, "JOYSTICK_LEFT_ARROW",     Input::JOYSTICK_LEFT_ARROW);
    engine->RegisterEnumValue(gJoystickButton, "JOYSTICK_RIGHT_ARROW",    Input::JOYSTICK_RIGHT_ARROW);
    engine->RegisterEnumValue(gJoystickButton, "JOYSTICK_START",          Input::JOYSTICK_START);
    engine->RegisterEnumValue(gJoystickButton, "JOYSTICK_BACK",           Input::JOYSTICK_BACK);
    engine->RegisterEnumValue(gJoystickButton, "JOYSTICK_LEFT_THUMB",     Input::JOYSTICK_LEFT_THUMB);
    engine->RegisterEnumValue(gJoystickButton, "JOYSTICK_RIGHT_THUMB",    Input::JOYSTICK_RIGHT_THUMB);
    engine->RegisterEnumValue(gJoystickButton, "JOYSTICK_LEFT_SHOULDER",  Input::JOYSTICK_LEFT_SHOULDER);
    engine->RegisterEnumValue(gJoystickButton, "JOYSTICK_RIGHT_SHOULDER", Input::JOYSTICK_RIGHT_SHOULDER);
    engine->RegisterEnumValue(gJoystickButton, "JOYSTICK_A",              Input::JOYSTICK_A);
    engine->RegisterEnumValue(gJoystickButton, "JOYSTICK_B",              Input::JOYSTICK_B);
    engine->RegisterEnumValue(gJoystickButton, "JOYSTICK_X",              Input::JOYSTICK_X);
    engine->RegisterEnumValue(gJoystickButton, "JOYSTICK_Y",              Input::JOYSTICK_Y);

    engine->RegisterEnum(gTouchState);
    engine->RegisterEnumValue(gTouchState, "TOUCH_HOVER",        Input::TOUCH_HOVER);
    engine->RegisterEnumValue(gTouchState, "TOUCH_BEGAN",        Input::TOUCH_BEGAN);
    engine->RegisterEnumValue(gTouchState, "TOUCH_MOVED",        Input::TOUCH_MOVED);
    engine->RegisterEnumValue(gTouchState, "TOUCH_ENDED",        Input::TOUCH_ENDED);
    engine->RegisterEnumValue(gTouchState, "TOUCH_CANCELLED",    Input::TOUCH_CANCELLED);

    engine->RegisterEnum(gCursorShape);
    engine->RegisterEnumValue(gCursorShape, "TOUCH_HOVER",        Input::CURSOR_ARROW);
    engine->RegisterEnumValue(gCursorShape, "CURSOR_CROSS",       Input::CURSOR_CROSS);
    engine->RegisterEnumValue(gCursorShape, "CURSOR_IBEAM",       Input::CURSOR_IBEAM);
    engine->RegisterEnumValue(gCursorShape, "CURSOR_HAND",        Input::CURSOR_HAND);
    engine->RegisterEnumValue(gCursorShape, "CURSOR_HORSIZE",     Input::CURSOR_HORSIZE);
    engine->RegisterEnumValue(gCursorShape, "CURSOR_VERSIZE",     Input::CURSOR_VERSIZE);
    engine->RegisterEnumValue(gCursorShape, "CURSOR_FDIAGSIZE",   Input::CURSOR_FDIAGSIZE);
    engine->RegisterEnumValue(gCursorShape, "CURSOR_BDIAGSIZE",   Input::CURSOR_BDIAGSIZE);
    engine->RegisterEnumValue(gCursorShape, "CURSOR_ALLSIZE",     Input::CURSOR_ALLSIZE);

    engine->RegisterGlobalFunction("bool isKey(int)",
                                   generic ? WRAP_FN(Input::isKey) : asFUNCTION(Input::isKey),
                                   generic ? asCALL_GENERIC : asCALL_CDECL);

    engine->RegisterGlobalFunction("bool isKeyDown(int)",
                                   generic ? WRAP_FN(Input::isKeyDown) : asFUNCTION(Input::isKeyDown),
                                   generic ? asCALL_GENERIC : asCALL_CDECL);

    engine->RegisterGlobalFunction("bool isKeyUp(int)",
                                   generic ? WRAP_FN(Input::isKeyUp) : asFUNCTION(Input::isKeyUp),
                                   generic ? asCALL_GENERIC : asCALL_CDECL);


    engine->RegisterGlobalFunction("string inputString()",
                                   generic ? WRAP_FN(Input::inputString) : asFUNCTION(Input::inputString),
                                   generic ? asCALL_GENERIC : asCALL_CDECL);

    engine->RegisterGlobalFunction("void setKeyboardVisible(bool)",
                                   generic ? WRAP_FN(Input::setKeyboardVisible) : asFUNCTION(Input::setKeyboardVisible),
                                   generic ? asCALL_GENERIC : asCALL_CDECL);


    engine->RegisterGlobalFunction("bool isMouseButton(int)",
                                   generic ? WRAP_FN(Input::isMouseButton) : asFUNCTION(Input::isMouseButton),
                                   generic ? asCALL_GENERIC : asCALL_CDECL);

    engine->RegisterGlobalFunction("bool isMouseButtonDown(int)",
                                   generic ? WRAP_FN(Input::isMouseButtonDown) : asFUNCTION(Input::isMouseButtonDown),
                                   generic ? asCALL_GENERIC : asCALL_CDECL);

    engine->RegisterGlobalFunction("bool isMouseButtonUp(int)",
                                   generic ? WRAP_FN(Input::isMouseButtonUp) : asFUNCTION(Input::isMouseButtonUp),
                                   generic ? asCALL_GENERIC : asCALL_CDECL);

    engine->RegisterGlobalFunction("Vector4 mousePosition()",
                                   generic ? WRAP_FN(Input::mousePosition) : asFUNCTION(Input::mousePosition),
                                   generic ? asCALL_GENERIC : asCALL_CDECL);

    engine->RegisterGlobalFunction("Vector4 mouseDelta()",
                                   generic ? WRAP_FN(Input::mouseDelta) : asFUNCTION(Input::mouseDelta),
                                   generic ? asCALL_GENERIC : asCALL_CDECL);

    engine->RegisterGlobalFunction("float mouseScrollDelta()",
                                   generic ? WRAP_FN(Input::mouseScrollDelta) : asFUNCTION(Input::mouseScrollDelta),
                                   generic ? asCALL_GENERIC : asCALL_CDECL);

    engine->RegisterGlobalFunction("void mouseLockCursor(bool)",
                                   generic ? WRAP_FN(Input::mouseLockCursor) : asFUNCTION(Input::mouseLockCursor),
                                   generic ? asCALL_GENERIC : asCALL_CDECL);

    engine->RegisterGlobalFunction("void mouseSetCursor(int)",
                                   generic ? WRAP_FN(Input::mouseSetCursor) : asFUNCTION(Input::mouseSetCursor),
                                   generic ? asCALL_GENERIC : asCALL_CDECL);


    engine->RegisterGlobalFunction("int joystickCount()",
                                   generic ? WRAP_FN(Input::joystickCount) : asFUNCTION(Input::joystickCount),
                                   generic ? asCALL_GENERIC : asCALL_CDECL);

    engine->RegisterGlobalFunction("int joystickButtons(int)",
                                   generic ? WRAP_FN(Input::joystickButtons) : asFUNCTION(Input::joystickButtons),
                                   generic ? asCALL_GENERIC : asCALL_CDECL);

    engine->RegisterGlobalFunction("Vector4 joystickThumbs(int)",
                                   generic ? WRAP_FN(Input::joystickThumbs) : asFUNCTION(Input::joystickThumbs),
                                   generic ? asCALL_GENERIC : asCALL_CDECL);

    engine->RegisterGlobalFunction("Vector2 joystickTriggers(int)",
                                   generic ? WRAP_FN(Input::joystickTriggers) : asFUNCTION(Input::joystickTriggers),
                                   generic ? asCALL_GENERIC : asCALL_CDECL);


    engine->RegisterGlobalFunction("int touchCount()",
                                   generic ? WRAP_FN(Input::touchCount) : asFUNCTION(Input::touchCount),
                                   generic ? asCALL_GENERIC : asCALL_CDECL);

    engine->RegisterGlobalFunction("int touchState(int)",
                                   generic ? WRAP_FN(Input::touchState) : asFUNCTION(Input::touchState),
                                   generic ? asCALL_GENERIC : asCALL_CDECL);

    engine->RegisterGlobalFunction("Vector4 touchPosition(int)",
                                   generic ? WRAP_FN(Input::touchPosition) : asFUNCTION(Input::touchPosition),
                                   generic ? asCALL_GENERIC : asCALL_CDECL);

    engine->SetDefaultNamespace("");
}
