#include "bindings/angelbindings.h"

#include <angelscript.h>

#include <input.h>

const char *MouseButton("MouseButton");
const char *JoystickButton("JoystickButton");
const char *TouchState("TouchState");
const char *KeyCode("KeyCode");

void registerInput(asIScriptEngine *engine) {
    engine->SetDefaultNamespace("Input");

    engine->RegisterEnum(KeyCode);
    engine->RegisterEnumValue(KeyCode, "KEY_UNKNOWN",           Input::KEY_UNKNOWN);
    engine->RegisterEnumValue(KeyCode, "KEY_SPACE",             Input::KEY_SPACE);
    engine->RegisterEnumValue(KeyCode, "KEY_APOSTROPHE",        Input::KEY_APOSTROPHE);
    engine->RegisterEnumValue(KeyCode, "KEY_COMMA",             Input::KEY_COMMA);
    engine->RegisterEnumValue(KeyCode, "KEY_MINUS",             Input::KEY_MINUS);
    engine->RegisterEnumValue(KeyCode, "KEY_PERIOD",            Input::KEY_PERIOD);
    engine->RegisterEnumValue(KeyCode, "KEY_SLASH",             Input::KEY_SLASH);
    engine->RegisterEnumValue(KeyCode, "KEY_0",                 Input::KEY_0);
    engine->RegisterEnumValue(KeyCode, "KEY_1",                 Input::KEY_1);
    engine->RegisterEnumValue(KeyCode, "KEY_2",                 Input::KEY_2);
    engine->RegisterEnumValue(KeyCode, "KEY_3",                 Input::KEY_3);
    engine->RegisterEnumValue(KeyCode, "KEY_4",                 Input::KEY_4);
    engine->RegisterEnumValue(KeyCode, "KEY_5",                 Input::KEY_5);
    engine->RegisterEnumValue(KeyCode, "KEY_6",                 Input::KEY_6);
    engine->RegisterEnumValue(KeyCode, "KEY_7",                 Input::KEY_7);
    engine->RegisterEnumValue(KeyCode, "KEY_8",                 Input::KEY_8);
    engine->RegisterEnumValue(KeyCode, "KEY_9",                 Input::KEY_9);
    engine->RegisterEnumValue(KeyCode, "KEY_SEMICOLON",         Input::KEY_SEMICOLON);
    engine->RegisterEnumValue(KeyCode, "KEY_EQUAL",             Input::KEY_EQUAL);
    engine->RegisterEnumValue(KeyCode, "KEY_A",                 Input::KEY_A);
    engine->RegisterEnumValue(KeyCode, "KEY_B",                 Input::KEY_B);
    engine->RegisterEnumValue(KeyCode, "KEY_C",                 Input::KEY_C);
    engine->RegisterEnumValue(KeyCode, "KEY_D",                 Input::KEY_D);
    engine->RegisterEnumValue(KeyCode, "KEY_E",                 Input::KEY_E);
    engine->RegisterEnumValue(KeyCode, "KEY_F",                 Input::KEY_F);
    engine->RegisterEnumValue(KeyCode, "KEY_G",                 Input::KEY_G);
    engine->RegisterEnumValue(KeyCode, "KEY_H",                 Input::KEY_H);
    engine->RegisterEnumValue(KeyCode, "KEY_I",                 Input::KEY_I);
    engine->RegisterEnumValue(KeyCode, "KEY_J",                 Input::KEY_J);
    engine->RegisterEnumValue(KeyCode, "KEY_K",                 Input::KEY_K);
    engine->RegisterEnumValue(KeyCode, "KEY_L",                 Input::KEY_L);
    engine->RegisterEnumValue(KeyCode, "KEY_M",                 Input::KEY_M);
    engine->RegisterEnumValue(KeyCode, "KEY_N",                 Input::KEY_N);
    engine->RegisterEnumValue(KeyCode, "KEY_O",                 Input::KEY_O);
    engine->RegisterEnumValue(KeyCode, "KEY_P",                 Input::KEY_P);
    engine->RegisterEnumValue(KeyCode, "KEY_Q",                 Input::KEY_Q);
    engine->RegisterEnumValue(KeyCode, "KEY_R",                 Input::KEY_R);
    engine->RegisterEnumValue(KeyCode, "KEY_S",                 Input::KEY_S);
    engine->RegisterEnumValue(KeyCode, "KEY_T",                 Input::KEY_T);
    engine->RegisterEnumValue(KeyCode, "KEY_U",                 Input::KEY_U);
    engine->RegisterEnumValue(KeyCode, "KEY_V",                 Input::KEY_V);
    engine->RegisterEnumValue(KeyCode, "KEY_W",                 Input::KEY_W);
    engine->RegisterEnumValue(KeyCode, "KEY_X",                 Input::KEY_X);
    engine->RegisterEnumValue(KeyCode, "KEY_Y",                 Input::KEY_Y);
    engine->RegisterEnumValue(KeyCode, "KEY_Z",                 Input::KEY_Z);
    engine->RegisterEnumValue(KeyCode, "KEY_LEFT_BRACKET",      Input::KEY_LEFT_BRACKET);
    engine->RegisterEnumValue(KeyCode, "KEY_BACKSLASH",         Input::KEY_BACKSLASH);
    engine->RegisterEnumValue(KeyCode, "KEY_RIGHT_BRACKET",     Input::KEY_RIGHT_BRACKET);
    engine->RegisterEnumValue(KeyCode, "KEY_GRAVE_ACCENT",      Input::KEY_GRAVE_ACCENT);
    engine->RegisterEnumValue(KeyCode, "KEY_WORLD_1",           Input::KEY_WORLD_1);
    engine->RegisterEnumValue(KeyCode, "KEY_WORLD_2",           Input::KEY_WORLD_2);
    engine->RegisterEnumValue(KeyCode, "KEY_ESCAPE",            Input::KEY_ESCAPE);
    engine->RegisterEnumValue(KeyCode, "KEY_ENTER",             Input::KEY_ENTER);
    engine->RegisterEnumValue(KeyCode, "KEY_TAB",               Input::KEY_TAB);
    engine->RegisterEnumValue(KeyCode, "KEY_BACKSPACE",         Input::KEY_BACKSPACE);
    engine->RegisterEnumValue(KeyCode, "KEY_INSERT",            Input::KEY_INSERT);
    engine->RegisterEnumValue(KeyCode, "KEY_DELETE",            Input::KEY_DELETE);
    engine->RegisterEnumValue(KeyCode, "KEY_RIGHT",             Input::KEY_RIGHT);
    engine->RegisterEnumValue(KeyCode, "KEY_LEFT",              Input::KEY_LEFT);
    engine->RegisterEnumValue(KeyCode, "KEY_DOWN",              Input::KEY_DOWN);
    engine->RegisterEnumValue(KeyCode, "KEY_UP",                Input::KEY_UP);
    engine->RegisterEnumValue(KeyCode, "KEY_PAGE_UP",           Input::KEY_PAGE_UP);
    engine->RegisterEnumValue(KeyCode, "KEY_PAGE_DOWN",         Input::KEY_PAGE_DOWN);
    engine->RegisterEnumValue(KeyCode, "KEY_HOME",              Input::KEY_HOME);
    engine->RegisterEnumValue(KeyCode, "KEY_END",               Input::KEY_END);
    engine->RegisterEnumValue(KeyCode, "KEY_CAPS_LOCK",         Input::KEY_CAPS_LOCK);
    engine->RegisterEnumValue(KeyCode, "KEY_SCROLL_LOCK",       Input::KEY_SCROLL_LOCK);
    engine->RegisterEnumValue(KeyCode, "KEY_NUM_LOCK",          Input::KEY_NUM_LOCK);
    engine->RegisterEnumValue(KeyCode, "KEY_PRINT_SCREEN",      Input::KEY_PRINT_SCREEN);
    engine->RegisterEnumValue(KeyCode, "KEY_PAUSE",             Input::KEY_PAUSE);
    engine->RegisterEnumValue(KeyCode, "KEY_F1",                Input::KEY_F1);
    engine->RegisterEnumValue(KeyCode, "KEY_F2",                Input::KEY_F2);
    engine->RegisterEnumValue(KeyCode, "KEY_F3",                Input::KEY_F3);
    engine->RegisterEnumValue(KeyCode, "KEY_F4",                Input::KEY_F4);
    engine->RegisterEnumValue(KeyCode, "KEY_F5",                Input::KEY_F5);
    engine->RegisterEnumValue(KeyCode, "KEY_F6",                Input::KEY_F6);
    engine->RegisterEnumValue(KeyCode, "KEY_F7",                Input::KEY_F7);
    engine->RegisterEnumValue(KeyCode, "KEY_F8",                Input::KEY_F8);
    engine->RegisterEnumValue(KeyCode, "KEY_F9",                Input::KEY_F9);
    engine->RegisterEnumValue(KeyCode, "KEY_F10",               Input::KEY_F10);
    engine->RegisterEnumValue(KeyCode, "KEY_F11",               Input::KEY_F11);
    engine->RegisterEnumValue(KeyCode, "KEY_F12",               Input::KEY_F12);
    engine->RegisterEnumValue(KeyCode, "KEY_F13",               Input::KEY_F13);
    engine->RegisterEnumValue(KeyCode, "KEY_F14",               Input::KEY_F14);
    engine->RegisterEnumValue(KeyCode, "KEY_F15",               Input::KEY_F15);
    engine->RegisterEnumValue(KeyCode, "KEY_F16",               Input::KEY_F16);
    engine->RegisterEnumValue(KeyCode, "KEY_F17",               Input::KEY_F17);
    engine->RegisterEnumValue(KeyCode, "KEY_F18",               Input::KEY_F18);
    engine->RegisterEnumValue(KeyCode, "KEY_F19",               Input::KEY_F19);
    engine->RegisterEnumValue(KeyCode, "KEY_F20",               Input::KEY_F20);
    engine->RegisterEnumValue(KeyCode, "KEY_F21",               Input::KEY_F21);
    engine->RegisterEnumValue(KeyCode, "KEY_F22",               Input::KEY_F22);
    engine->RegisterEnumValue(KeyCode, "KEY_F23",               Input::KEY_F23);
    engine->RegisterEnumValue(KeyCode, "KEY_F24",               Input::KEY_F24);
    engine->RegisterEnumValue(KeyCode, "KEY_F25",               Input::KEY_F25);
    engine->RegisterEnumValue(KeyCode, "KEY_KP_0",              Input::KEY_KP_0);
    engine->RegisterEnumValue(KeyCode, "KEY_KP_1",              Input::KEY_KP_1);
    engine->RegisterEnumValue(KeyCode, "KEY_KP_2",              Input::KEY_KP_2);
    engine->RegisterEnumValue(KeyCode, "KEY_KP_3",              Input::KEY_KP_3);
    engine->RegisterEnumValue(KeyCode, "KEY_KP_4",              Input::KEY_KP_4);
    engine->RegisterEnumValue(KeyCode, "KEY_KP_5",              Input::KEY_KP_5);
    engine->RegisterEnumValue(KeyCode, "KEY_KP_6",              Input::KEY_KP_6);
    engine->RegisterEnumValue(KeyCode, "KEY_KP_7",              Input::KEY_KP_7);
    engine->RegisterEnumValue(KeyCode, "KEY_KP_8",              Input::KEY_KP_8);
    engine->RegisterEnumValue(KeyCode, "KEY_KP_9",              Input::KEY_KP_9);
    engine->RegisterEnumValue(KeyCode, "KEY_KP_DECIMAL",        Input::KEY_KP_DECIMAL);
    engine->RegisterEnumValue(KeyCode, "KEY_KP_DIVIDE",         Input::KEY_KP_DIVIDE);
    engine->RegisterEnumValue(KeyCode, "KEY_KP_MULTIPLY",       Input::KEY_KP_MULTIPLY);
    engine->RegisterEnumValue(KeyCode, "KEY_KP_SUBTRACT",       Input::KEY_KP_SUBTRACT);
    engine->RegisterEnumValue(KeyCode, "KEY_KP_ADD",            Input::KEY_KP_ADD);
    engine->RegisterEnumValue(KeyCode, "KEY_KP_ENTER",          Input::KEY_KP_ENTER);
    engine->RegisterEnumValue(KeyCode, "KEY_KP_EQUAL",          Input::KEY_KP_EQUAL);
    engine->RegisterEnumValue(KeyCode, "KEY_LEFT_SHIFT",        Input::KEY_LEFT_SHIFT);
    engine->RegisterEnumValue(KeyCode, "KEY_LEFT_CONTROL",      Input::KEY_LEFT_CONTROL);
    engine->RegisterEnumValue(KeyCode, "KEY_LEFT_ALT",          Input::KEY_LEFT_ALT);
    engine->RegisterEnumValue(KeyCode, "KEY_LEFT_SUPER",          Input::KEY_LEFT_SUPER);
    engine->RegisterEnumValue(KeyCode, "KEY_RIGHT_SHIFT",       Input::KEY_RIGHT_SHIFT);
    engine->RegisterEnumValue(KeyCode, "KEY_RIGHT_CONTROL",     Input::KEY_RIGHT_CONTROL);
    engine->RegisterEnumValue(KeyCode, "KEY_RIGHT_ALT",         Input::KEY_RIGHT_ALT);
    engine->RegisterEnumValue(KeyCode, "KEY_RIGHT_SUPER",         Input::KEY_RIGHT_SUPER);
    engine->RegisterEnumValue(KeyCode, "KEY_MENU",              Input::KEY_MENU);

    engine->RegisterEnumValue(KeyCode, "MOUSE_LEFT",      Input::MOUSE_LEFT);
    engine->RegisterEnumValue(KeyCode, "MOUSE_RIGHT",     Input::MOUSE_RIGHT);
    engine->RegisterEnumValue(KeyCode, "MOUSE_MIDDLE",    Input::MOUSE_MIDDLE);
    engine->RegisterEnumValue(KeyCode, "MOUSE_BUTTON0",   Input::MOUSE_BUTTON0);
    engine->RegisterEnumValue(KeyCode, "MOUSE_BUTTON1",   Input::MOUSE_BUTTON1);
    engine->RegisterEnumValue(KeyCode, "MOUSE_BUTTON2",   Input::MOUSE_BUTTON2);
    engine->RegisterEnumValue(KeyCode, "MOUSE_BUTTON3",   Input::MOUSE_BUTTON3);
    engine->RegisterEnumValue(KeyCode, "MOUSE_BUTTON4",   Input::MOUSE_BUTTON4);

    engine->RegisterEnumValue(KeyCode, "JOYSTICK_UP_ARROW",       Input::JOYSTICK_UP_ARROW);
    engine->RegisterEnumValue(KeyCode, "JOYSTICK_DOWN_ARROW",     Input::JOYSTICK_DOWN_ARROW);
    engine->RegisterEnumValue(KeyCode, "JOYSTICK_LEFT_ARROW",     Input::JOYSTICK_LEFT_ARROW);
    engine->RegisterEnumValue(KeyCode, "JOYSTICK_RIGHT_ARROW",    Input::JOYSTICK_RIGHT_ARROW);
    engine->RegisterEnumValue(KeyCode, "JOYSTICK_START",          Input::JOYSTICK_START);
    engine->RegisterEnumValue(KeyCode, "JOYSTICK_BACK",           Input::JOYSTICK_BACK);
    engine->RegisterEnumValue(KeyCode, "JOYSTICK_LEFT_THUMB",     Input::JOYSTICK_LEFT_THUMB);
    engine->RegisterEnumValue(KeyCode, "JOYSTICK_RIGHT_THUMB",    Input::JOYSTICK_RIGHT_THUMB);
    engine->RegisterEnumValue(KeyCode, "JOYSTICK_LEFT_SHOULDER",  Input::JOYSTICK_LEFT_SHOULDER);
    engine->RegisterEnumValue(KeyCode, "JOYSTICK_RIGHT_SHOULDER", Input::JOYSTICK_RIGHT_SHOULDER);
    engine->RegisterEnumValue(KeyCode, "JOYSTICK_A",              Input::JOYSTICK_A);
    engine->RegisterEnumValue(KeyCode, "JOYSTICK_B",              Input::JOYSTICK_B);
    engine->RegisterEnumValue(KeyCode, "JOYSTICK_X",              Input::JOYSTICK_X);
    engine->RegisterEnumValue(KeyCode, "JOYSTICK_Y",              Input::JOYSTICK_Y);

    engine->RegisterEnumValue(KeyCode, "TOUCH_HOVER",        Input::TOUCH_HOVER);
    engine->RegisterEnumValue(KeyCode, "TOUCH_BEGAN",        Input::TOUCH_BEGAN);
    engine->RegisterEnumValue(KeyCode, "TOUCH_MOVED",        Input::TOUCH_MOVED);
    engine->RegisterEnumValue(KeyCode, "TOUCH_ENDED",        Input::TOUCH_ENDED);
    engine->RegisterEnumValue(KeyCode, "TOUCH_CANCELLED",    Input::TOUCH_CANCELLED);

    engine->RegisterGlobalFunction("bool isKey(int)", asFUNCTION(Input::isKey), asCALL_CDECL);
    engine->RegisterGlobalFunction("bool isKeyDown(int)", asFUNCTION(Input::isKeyDown), asCALL_CDECL);
    engine->RegisterGlobalFunction("bool isKeyUp(int)", asFUNCTION(Input::isKeyUp), asCALL_CDECL);

    engine->RegisterGlobalFunction("bool isMouseButton(int)", asFUNCTION(Input::isMouseButton), asCALL_CDECL);
    engine->RegisterGlobalFunction("bool isMouseButtonDown(int)", asFUNCTION(Input::isMouseButtonDown), asCALL_CDECL);
    engine->RegisterGlobalFunction("bool isMouseButtonUp(int)", asFUNCTION(Input::isMouseButtonUp), asCALL_CDECL);

    engine->RegisterGlobalFunction("Vector4 mousePosition()", asFUNCTION(Input::mousePosition), asCALL_CDECL);
    engine->RegisterGlobalFunction("Vector4 mouseDelta()", asFUNCTION(Input::mouseDelta), asCALL_CDECL);
    engine->RegisterGlobalFunction("void mouseLockCursor(bool)", asFUNCTION(Input::mouseLockCursor), asCALL_CDECL);

    engine->RegisterGlobalFunction("int joystickCount()", asFUNCTION(Input::joystickCount), asCALL_CDECL);
    engine->RegisterGlobalFunction("int joystickButtons(int)", asFUNCTION(Input::joystickButtons), asCALL_CDECL);
    engine->RegisterGlobalFunction("Vector4 joystickThumbs(int)", asFUNCTION(Input::joystickThumbs), asCALL_CDECL);
    engine->RegisterGlobalFunction("Vector2 joystickTriggers(int)", asFUNCTION(Input::joystickTriggers), asCALL_CDECL);

    engine->RegisterGlobalFunction("int touchCount()", asFUNCTION(Input::touchCount), asCALL_CDECL);
    engine->RegisterGlobalFunction("int touchState(int)", asFUNCTION(Input::touchState), asCALL_CDECL);
    engine->RegisterGlobalFunction("Vector4 touchPosition(int)", asFUNCTION(Input::touchPosition), asCALL_CDECL);

    engine->SetDefaultNamespace("");
}
