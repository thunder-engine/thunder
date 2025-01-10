#include "adapters/mobileadaptor.h"

#include <glfm.h>

#include "log.h"
#include "file.h"

#include "engine.h"
#include "input.h"
#include "timer.h"

#ifdef __ANDROID__
#include "androidfile.h"
#include <android/log.h>

class AndroidHandler : public LogHandler {
protected:
    void setRecord (Log::LogTypes type, const char *record) {
        int32_t lvl;
        switch(type) {
            case Log::ERR: lvl = ANDROID_LOG_ERROR; break;
            case Log::WRN: lvl = ANDROID_LOG_WARN; break;
            case Log::INF: lvl = ANDROID_LOG_INFO; break;
            case Log::DBG: lvl = ANDROID_LOG_DEBUG; break;
            default: break;
        }
        __android_log_write(lvl, "ThunderEngine", record);
    }
};
#else

#ifdef __EMSCRIPTEN__
#include "emscriptenfile.h"
#endif

const char *configLocation();
const char *assetsLocation();

class DefaultHandler : public LogHandler {
protected:
    void setRecord(Log::LogTypes type, const char *record) {
        const char *lvl;
        switch(type) {
            case Log::ERR: lvl = "ERROR"; break;
            case Log::WRN: lvl = "WARNING"; break;
            case Log::INF: lvl = "INFO"; break;
            case Log::DBG: lvl = "DEBUG"; break;
            default: break;
        }
        printf("[%s] %s\n", lvl, record);
    }
};
#endif

#define NONE -1
#define RELEASE 0
#define PRESS 1
#define REPEAT 2

static GLFMDisplay *s_display = nullptr;
static Engine *s_engine = nullptr;

static std::unordered_map<int32_t, int32_t> s_keys;
static std::unordered_map<int32_t, std::pair<uint32_t, Vector4>> s_touches;

static std::string s_inputString;

Vector4 MobileAdaptor::s_mousePosition = Vector4();
Vector4 MobileAdaptor::s_oldMousePosition = Vector4();

Vector4 MobileAdaptor::s_thumbs = Vector4();
int32_t MobileAdaptor::s_width = 0;
int32_t MobileAdaptor::s_height = 0;
float MobileAdaptor::s_mouseScrollDelta = 0.0f;
bool MobileAdaptor::s_mouseLocked = false;

int keyToInput(int key) {
    int result = key;

    switch(key) {
        case GLFMKeyCodeBackspace: result = Input::KEY_BACKSPACE; break;
        case GLFMKeyCodeTab: result = Input::KEY_TAB; break;
        case GLFMKeyCodeEnter: result = Input::KEY_ENTER; break;
        case GLFMKeyCodeEscape: result = Input::KEY_ESCAPE; break;
        case GLFMKeyCodeDelete: result = Input::KEY_DELETE; break;

        case GLFMKeyCodeCapsLock: result = Input::KEY_CAPS_LOCK; break;
        case GLFMKeyCodeShiftLeft: result = Input::KEY_LEFT_SHIFT; break;
        case GLFMKeyCodeShiftRight: result = Input::KEY_RIGHT_SHIFT; break;
        case GLFMKeyCodeControlLeft: result = Input::KEY_LEFT_CONTROL; break;
        case GLFMKeyCodeControlRight: result = Input::KEY_RIGHT_CONTROL; break;
        case GLFMKeyCodeAltLeft: result = Input::KEY_LEFT_ALT; break;
        case GLFMKeyCodeAltRight: result = Input::KEY_RIGHT_ALT; break;
        case GLFMKeyCodeMetaLeft: result = Input::KEY_LEFT_SUPER; break;
        case GLFMKeyCodeMetaRight: result = Input::KEY_RIGHT_SUPER; break;
        case GLFMKeyCodeMenu: result = Input::KEY_MENU; break;

        case GLFMKeyCodeInsert: result = Input::KEY_INSERT; break;
        case GLFMKeyCodePageUp: result = Input::KEY_PAGE_UP; break;
        case GLFMKeyCodePageDown: result = Input::KEY_PAGE_DOWN; break;
        case GLFMKeyCodeEnd: result = Input::KEY_END; break;
        case GLFMKeyCodeHome: result = Input::KEY_HOME; break;
        case GLFMKeyCodeArrowLeft: result = Input::KEY_LEFT; break;
        case GLFMKeyCodeArrowUp: result = Input::KEY_UP; break;
        case GLFMKeyCodeArrowRight: result = Input::KEY_RIGHT; break;
        case GLFMKeyCodeArrowDown: result = Input::KEY_DOWN; break;

        //case GLFMKeyCodePower: result =
        //case GLFMKeyCodeFunction: result =
        case GLFMKeyCodePrintScreen: result = Input::KEY_PRINT_SCREEN; break;
        case GLFMKeyCodeScrollLock: result = Input::KEY_SCROLL_LOCK; break;
        case GLFMKeyCodePause: result = Input::KEY_PAUSE; break;

        case GLFMKeyCodeNumLock: result = Input::KEY_NUM_LOCK; break;
        case GLFMKeyCodeNumpadDecimal: result = Input::KEY_KP_DECIMAL; break;
        case GLFMKeyCodeNumpadMultiply: result = Input::KEY_KP_MULTIPLY; break;
        case GLFMKeyCodeNumpadAdd: result = Input::KEY_KP_ADD; break;
        case GLFMKeyCodeNumpadDivide: result = Input::KEY_KP_DIVIDE; break;
        case GLFMKeyCodeNumpadEnter: result = Input::KEY_KP_ENTER; break;
        case GLFMKeyCodeNumpadSubtract: result = Input::KEY_KP_SUBTRACT; break;
        case GLFMKeyCodeNumpadEqual: result = Input::KEY_KP_EQUAL; break;

        case GLFMKeyCodeNumpad0: result = Input::KEY_KP_0; break;
        case GLFMKeyCodeNumpad1: result = Input::KEY_KP_1; break;
        case GLFMKeyCodeNumpad2: result = Input::KEY_KP_2; break;
        case GLFMKeyCodeNumpad3: result = Input::KEY_KP_3; break;
        case GLFMKeyCodeNumpad4: result = Input::KEY_KP_4; break;
        case GLFMKeyCodeNumpad5: result = Input::KEY_KP_5; break;
        case GLFMKeyCodeNumpad6: result = Input::KEY_KP_6; break;
        case GLFMKeyCodeNumpad7: result = Input::KEY_KP_7; break;
        case GLFMKeyCodeNumpad8: result = Input::KEY_KP_8; break;
        case GLFMKeyCodeNumpad9: result = Input::KEY_KP_9; break;

        case GLFMKeyCodeF1: result = Input::KEY_F1; break;
        case GLFMKeyCodeF2: result = Input::KEY_F2; break;
        case GLFMKeyCodeF3: result = Input::KEY_F3; break;
        case GLFMKeyCodeF4: result = Input::KEY_F4; break;
        case GLFMKeyCodeF5: result = Input::KEY_F5; break;
        case GLFMKeyCodeF6: result = Input::KEY_F6; break;
        case GLFMKeyCodeF7: result = Input::KEY_F7; break;
        case GLFMKeyCodeF8: result = Input::KEY_F8; break;
        case GLFMKeyCodeF9: result = Input::KEY_F9; break;
        case GLFMKeyCodeF10: result = Input::KEY_F10; break;
        case GLFMKeyCodeF11: result = Input::KEY_F11; break;
        case GLFMKeyCodeF12: result = Input::KEY_F12; break;
        case GLFMKeyCodeF13: result = Input::KEY_F13; break;
        case GLFMKeyCodeF14: result = Input::KEY_F14; break;
        case GLFMKeyCodeF15: result = Input::KEY_F15; break;
        case GLFMKeyCodeF16: result = Input::KEY_F16; break;
        case GLFMKeyCodeF17: result = Input::KEY_F17; break;
        case GLFMKeyCodeF18: result = Input::KEY_F18; break;
        case GLFMKeyCodeF19: result = Input::KEY_F19; break;
        case GLFMKeyCodeF20: result = Input::KEY_F20; break;
        case GLFMKeyCodeF21: result = Input::KEY_F21; break;
        case GLFMKeyCodeF22: result = Input::KEY_F22; break;
        case GLFMKeyCodeF23: result = Input::KEY_F23; break;
        case GLFMKeyCodeF24: result = Input::KEY_F24; break;

        case GLFMKeyCodeNavigationBack: result = Input::JOYSTICK_BACK; break;
        case GLFMKeyCodeMediaSelect: result = Input::JOYSTICK_A; break;
        case GLFMKeyCodeMediaPlayPause: result = Input::JOYSTICK_START; break;
        default: break;
    }

    return result;
}

void onFrame(GLFMDisplay *display) {
    if(s_engine) {
        Timer::update();

        s_engine->update();
    }

    glfmSwapBuffers(display);
}

void onCreate(GLFMDisplay *, int width, int height) {
    MobileAdaptor::s_width = width;
    MobileAdaptor::s_height = height;

    File *file = nullptr;
    const char *path = "";
#ifdef __ANDROID__
    Log::overrideHandler(new AndroidHandler());
    file = new AndroidFile();
#else
    Log::overrideHandler(new DefaultHandler());
    #ifdef __EMSCRIPTEN__
        file = new EmscriptenFile();
    #else
        file = new File();
        file->finit(path);
        file->fsearchPathAdd(assetsLocation());
    #endif
#endif

    s_engine = new Engine(file, path);

    thunderMain(s_engine);
}

void onResize(GLFMDisplay *, int width, int height) {
    MobileAdaptor::s_width = width;
    MobileAdaptor::s_height = height;
}

bool onTouch(GLFMDisplay *, int touch, GLFMTouchPhase phase, double x, double y) {
#ifdef GLFM_PLATFORM_TVOS
    if(phase < GLFMTouchPhaseEnded) {
        MobileAdaptor::s_thumbs.z = MobileAdaptor::s_thumbs.x = CLAMP(x / (float)MobileAdaptor::s_width - 0.5f, -1.0f, 1.0f);
        MobileAdaptor::s_thumbs.w = MobileAdaptor::s_thumbs.y = CLAMP(y / (float)MobileAdaptor::s_height - 0.5f, -1.0f, 1.0f);
    } else {
        MobileAdaptor::s_thumbs = Vector4(0.0f);
    }
#else
    if(phase < GLFMTouchPhaseCancelled) {
        Vector4 pos(x, MobileAdaptor::s_height - y,
                    x / (float)MobileAdaptor::s_width, (MobileAdaptor::s_height - y) / (float)MobileAdaptor::s_width);

        if(touch == 0) {
            MobileAdaptor::s_mousePosition = pos;
        }

        int index = touch;
        switch(touch) {
            case 0: index = Input::MOUSE_LEFT; break;
            case 1: index = Input::MOUSE_MIDDLE; break;
            case 2: index = Input::MOUSE_RIGHT; break;
            default: break;
        }

        int state = NONE;
        switch(phase) {
            case GLFMTouchPhaseBegan: state = PRESS; break;
            case GLFMTouchPhaseMoved: state = REPEAT; break;
            case GLFMTouchPhaseEnded: state = RELEASE; break;
            default: break;
        }

        s_touches[index] = std::make_pair(state, pos);
    } else {
        auto it = s_touches.find(touch);
        if(it != s_touches.end()) {
            s_touches.erase(it);
        }
    }
#endif
    return true;
}

bool onKey(GLFMDisplay *, GLFMKeyCode keyCode, GLFMKeyAction action, int) {
    int state = NONE;
    switch(action) {
        case GLFMKeyActionPressed: state = PRESS; break;
        case GLFMKeyActionRepeated: state = REPEAT; break;
        case GLFMKeyActionReleased: state = RELEASE; break;
        default: break;
    }

    s_keys[keyToInput(keyCode)] = state;

    return true;
}

bool onMouseWeel(GLFMDisplay *, double x, double y, GLFMMouseWheelDeltaType deltaType,
                 double, double deltaY, double) {

    MobileAdaptor::s_mousePosition = Vector4(x, MobileAdaptor::s_height - y,
                                             x / (float)MobileAdaptor::s_width, (MobileAdaptor::s_height - y) / (float)MobileAdaptor::s_width);

    MobileAdaptor::s_mouseScrollDelta = deltaY;

    return true;
}

void onChar(GLFMDisplay *, const char *utf8, int) {
    s_inputString += utf8;
}

void glfmMain(GLFMDisplay *display) {
    s_display = display;

    glfmSetDisplayConfig(s_display,
                         GLFMRenderingAPIOpenGLES3,
                         GLFMColorFormatRGBA8888,
                         GLFMDepthFormat16,
                         GLFMStencilFormatNone,
                         GLFMMultisampleNone);

    glfmSetRenderFunc(s_display, onFrame);
    glfmSetSurfaceCreatedFunc(s_display, onCreate);
    glfmSetSurfaceResizedFunc(s_display, onResize);

    glfmSetTouchFunc(s_display, onTouch);
    glfmSetKeyFunc(s_display, onKey);
    glfmSetCharFunc(s_display, onChar);

    glfmSetMouseWheelFunc(s_display, onMouseWeel);
}

MobileAdaptor::MobileAdaptor() {

}

bool MobileAdaptor::init() {
    return true;
}

void MobileAdaptor::update() {
    s_inputString.clear();

    for(auto &it : s_keys) {
        switch(it.second) {
            case RELEASE: it.second = NONE; break;
            case PRESS: it.second = REPEAT; break;
            default: break;
        }
    }

    auto it = s_touches.begin();
    while(it != s_touches.end()) {
        if(it->second.first == RELEASE) {
            it = s_touches.erase(it);
        } else {
            if(it->second.first == PRESS) {
                it->second.first = REPEAT;
            }

            ++it;
        }
    }

    MobileAdaptor::s_oldMousePosition = MobileAdaptor::s_mousePosition;

    MobileAdaptor::s_mouseScrollDelta = 0.0f;

    PlatformAdaptor::update();
}

bool MobileAdaptor::start() {
    Engine::reloadBundle();

    return true;
}

void MobileAdaptor::stop() {

}

void MobileAdaptor::destroy() {

}

bool MobileAdaptor::isValid() {
    return true;
}

std::string MobileAdaptor::locationLocalDir() const {
#ifdef __ANDROID__
    return reinterpret_cast<ANativeActivity *>(glfmGetAndroidActivity(s_display))->internalDataPath;
#else
    return configLocation();
#endif
}

uint32_t MobileAdaptor::screenWidth() const {
    return s_width;
}

uint32_t MobileAdaptor::screenHeight() const {
    return s_height;
}

bool MobileAdaptor::key(Input::KeyCode code) const {
    auto it = s_keys.find(code);
    if(it != s_keys.end()) {
        return it->second == PRESS || it->second == REPEAT;
    }
    return false;
}

bool MobileAdaptor::keyPressed(Input::KeyCode code) const {
    auto it = s_keys.find(code);
    if(it != s_keys.end()) {
        return it->second == PRESS;
    }
    return false;
}

bool MobileAdaptor::keyReleased(Input::KeyCode code) const {
    auto it = s_keys.find(code);
    if(it != s_keys.end()) {
        return it->second == RELEASE;
    }
    return false;
}

std::string MobileAdaptor::inputString() const {
    return s_inputString;
}

void MobileAdaptor::setKeyboardVisible(bool visible) {
#ifndef __EMSCRIPTEN__
    glfmSetKeyboardVisible(s_display, visible);
#endif
}

Vector4 MobileAdaptor::mousePosition() const {
    return MobileAdaptor::s_mousePosition;
}

Vector4 MobileAdaptor::mouseDelta() const {
    return MobileAdaptor::s_mousePosition - MobileAdaptor::s_oldMousePosition;
}

float MobileAdaptor::mouseScrollDelta() const {
    return MobileAdaptor::s_mouseScrollDelta;
}

bool MobileAdaptor::mouseButton(int button) const {
    auto it = s_touches.find(button);
    if(it != s_touches.end()) {
        return it->second.first == PRESS || it->second.first == REPEAT;
    }
    return false;
}

bool MobileAdaptor::mousePressed(int button) const {
    auto it = s_touches.find(button);
    if(it != s_touches.end()) {
        return it->second.first == PRESS;
    }
    return false;
}

bool MobileAdaptor::mouseReleased(int button) const {
    auto it = s_touches.find(button);
    if(it != s_touches.end()) {
        return it->second.first == RELEASE;
    }
    return false;
}

void MobileAdaptor::mouseLockCursor(bool lock) {
    s_mouseLocked = lock;
}

uint32_t MobileAdaptor::touchCount() const {
    int result = 0;
    for(auto it : s_touches) {
        if(it.second.first != NONE) {
            result++;
        }
    }
    return result;
}

uint32_t MobileAdaptor::touchState(int index) const {
    auto it = s_touches.find(index | Input::MOUSE_LEFT);
    if(it != s_touches.end()) {
        return it->second.first;
    }
    return 0;
}

Vector4 MobileAdaptor::touchPosition(int index) const {
    auto it = s_touches.find(index | Input::MOUSE_LEFT);
    if(it != s_touches.end()) {
        return it->second.second;
    }
    return Vector4();
}

uint32_t MobileAdaptor::joystickCount() const {
#ifdef GLFM_PLATFORM_TVOS
    return 1;
#else
    return 0;
#endif
}

uint32_t MobileAdaptor::joystickButtons(int index) const {
    return s_keys[index];
}

Vector4 MobileAdaptor::joystickThumbs(int) const {
    return s_thumbs;
}
