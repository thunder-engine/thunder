#include "adapters/mobileadaptor.h"

#include <glfm.h>

#include "log.h"
#include "file.h"

#include "engine.h"
#include "input.h"

#ifdef GLFM_PLATFORM_ANDROID
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

const char *configLocation();
const char *assetsLocation();

class AppleHandler : public LogHandler {
protected:
    void setRecord (Log::LogTypes type, const char *record) {
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

static GLFMDisplay *gDisplay = nullptr;
static Engine *g_pEngine = nullptr;

uint32_t MobileAdaptor::s_Buttons = 0;
Vector4 MobileAdaptor::s_Thumbs = Vector4();
Vector2 MobileAdaptor::s_Screen = Vector2();

static string s_inputString;

struct Touch {
    uint32_t    phase;
    Vector4     pos;
};
typedef map<int, Touch> TouchMap;
static TouchMap s_Touches;

void onFrame(GLFMDisplay *, const double) {
    if(g_pEngine) {
        g_pEngine->update();
    }
}

void onCreate(GLFMDisplay *, int width, int height) {
    MobileAdaptor::s_Screen = Vector2(width, height);

    File *file = nullptr;
    const char *path = "";
#ifdef GLFM_PLATFORM_ANDROID
    Log::overrideHandler(new AndroidHandler());
    file = new AndroidFile();
#else
    Log::overrideHandler(new AppleHandler());
    file = new File();
    file->finit(path);
    file->fsearchPathAdd(assetsLocation());
#endif

    g_pEngine = new Engine(file, path);

    thunderMain(g_pEngine);
}

void onResize(GLFMDisplay *, int width, int height) {
    MobileAdaptor::s_Screen = Vector2(width, height);
    if(g_pEngine) {
        g_pEngine->resize();
    }
}

bool onTouch(GLFMDisplay *, int touch, GLFMTouchPhase phase, double x, double y) {
#ifdef GLFM_PLATFORM_TVOS
    if(phase < GLFMTouchPhaseEnded) {
        MobileAdaptor::s_Thumbs.z = MobileAdaptor::s_Thumbs.x = CLAMP(x / MobileAdaptor::s_Screen.x - 0.5f, -1.0f, 1.0f);
        MobileAdaptor::s_Thumbs.w = MobileAdaptor::s_Thumbs.y = CLAMP(y / MobileAdaptor::s_Screen.y - 0.5f, -1.0f, 1.0f);
    } else {
        MobileAdaptor::s_Thumbs = Vector4(0.0f);
    }
#else
    if(phase < GLFMTouchPhaseEnded) {
        Touch t;
        t.phase = phase;
        t.pos = Vector4(x, MobileAdaptor::s_Screen.y - y, x / MobileAdaptor::s_Screen.x, (MobileAdaptor::s_Screen.y - y) / MobileAdaptor::s_Screen.y);
        s_Touches[touch] = t;
    } else {
        auto it = s_Touches.find(touch);
        if(it != s_Touches.end()) {
            s_Touches.erase(it);
        }
    }
#endif
    return true;
}

bool onKey(GLFMDisplay *, GLFMKey keyCode, GLFMKeyAction, int) {
    switch(keyCode) {
        case GLFMKeyNavSelect: {
            MobileAdaptor::s_Buttons ^= Input::JOYSTICK_A;
        } return true;
        case GLFMKeyPlayPause: {
            MobileAdaptor::s_Buttons ^= Input::JOYSTICK_X;
        } return true;
        case GLFMKeyLeft: {
            MobileAdaptor::s_Buttons ^= Input::JOYSTICK_LEFT_ARROW;
        } return true;
        case GLFMKeyUp: {
            MobileAdaptor::s_Buttons ^= Input::JOYSTICK_UP_ARROW;
        } return true;
        case GLFMKeyRight: {
            MobileAdaptor::s_Buttons ^= Input::JOYSTICK_RIGHT_ARROW;
        } return true;
        case GLFMKeyDown: {
            MobileAdaptor::s_Buttons ^= Input::JOYSTICK_DOWN_ARROW;
        } return true;
        default: break;
    }

    return false;
}

void onChar(GLFMDisplay *, const char *utf8, int) {
    s_inputString += utf8;
}

void glfmMain(GLFMDisplay *display) {
    gDisplay = display;

    glfmSetDisplayConfig(gDisplay,
                         GLFMRenderingAPIOpenGLES3,
                         GLFMColorFormatRGBA8888,
                         GLFMDepthFormat16,
                         GLFMStencilFormatNone,
                         GLFMMultisampleNone);

    glfmSetMainLoopFunc(gDisplay, onFrame);
    glfmSetSurfaceCreatedFunc(gDisplay, onCreate);
    glfmSetSurfaceResizedFunc(gDisplay, onResize);

    glfmSetTouchFunc(gDisplay, onTouch);
    glfmSetKeyFunc(gDisplay, onKey);
    glfmSetCharFunc(gDisplay, onChar);
}

MobileAdaptor::MobileAdaptor(Engine *engine) {
    g_pEngine = engine;
}

bool MobileAdaptor::init() {
    return true;
}

void MobileAdaptor::update() {
    s_inputString.clear();

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

string MobileAdaptor::locationLocalDir() const {
#ifdef GLFM_PLATFORM_ANDROID
    return glfmAndroidGetActivity()->internalDataPath;
#else
    return configLocation();
#endif
}

uint32_t MobileAdaptor::screenWidth() const {
    return s_Screen.x;
}

uint32_t MobileAdaptor::screenHeight() const {
    return s_Screen.y;
}

string MobileAdaptor::inputString() const {
    return s_inputString;
}

void MobileAdaptor::setKeyboardVisible(bool visible) {
    glfmSetKeyboardVisible(gDisplay, visible);
}

uint32_t MobileAdaptor::touchCount() const {
    return s_Touches.size();
}

uint32_t MobileAdaptor::touchState(uint32_t index) const {
    return s_Touches[index].phase;
}

Vector4 MobileAdaptor::touchPosition(uint32_t index) const {
    return s_Touches[index].pos;
}

uint32_t MobileAdaptor::joystickCount() const {
#ifdef GLFM_PLATFORM_TVOS
    return 1;
#else
    return 0;
#endif
}

uint32_t MobileAdaptor::joystickButtons(uint32_t) const {
    return s_Buttons;
}

Vector4 MobileAdaptor::joystickThumbs(uint32_t) const {
    return s_Thumbs;
}
