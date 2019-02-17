#include "adapters/mobileadaptor.h"

#include <glfm.h>

#include <log.h>
#include <file.h>

#include "engine.h"

#ifdef GLFM_PLATFORM_ANDROID

#include "androidfile.h"
#include <android/log.h>

class AndroidHandler : public ILogHandler {
protected:
    void setRecord (Log::LogTypes, const char *record) {
        __android_log_write(ANDROID_LOG_DEBUG, "ThunderEngine", record);
    }
};
#endif

static GLFMDisplay *gDisplay = nullptr;
static Engine *g_pEngine = nullptr;

Vector2 MobileAdaptor::s_Screen = Vector2();

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

    IFile *file = nullptr;
#ifdef GLFM_PLATFORM_ANDROID
    Log::overrideHandler(new AndroidHandler());
    file = new AndroidFile();
#endif

    char *path = "";
    thunderMain(new Engine(file, 1, &path));
}

void onResize(GLFMDisplay *, int width, int height) {
    MobileAdaptor::s_Screen = Vector2(width, height);
    if(g_pEngine) {
        g_pEngine->resize();
    }
}

bool onTouch(GLFMDisplay *, int touch, GLFMTouchPhase phase, double x, double y) {
    if(phase < GLFMTouchPhaseEnded) {
        Touch t;
        t.phase = phase;
        t.pos = Vector4(x, y, x / MobileAdaptor::s_Screen.x, y / MobileAdaptor::s_Screen.y);
        s_Touches[touch] = t;
    } else {
        auto it = s_Touches.find(touch);
        if(it != s_Touches.end()) {
            s_Touches.erase(it);
        }
    }
    return true;
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
}

MobileAdaptor::MobileAdaptor(Engine *engine) {
    g_pEngine = engine;
}

bool MobileAdaptor::init() {
    return true;
}

void MobileAdaptor::update() {

}

bool MobileAdaptor::start() {
    return true;
}

void MobileAdaptor::stop() {

}

void MobileAdaptor::destroy() {

}

bool MobileAdaptor::isValid() {
    return true;
}

uint32_t MobileAdaptor::screenWidth() {
    return s_Screen.x;
}

uint32_t MobileAdaptor::screenHeight() {
    return s_Screen.y;
}

uint16_t MobileAdaptor::touchCount() {
    return s_Touches.size();
}

uint16_t MobileAdaptor::touchState(uint8_t index) {
    return s_Touches[index].phase;
}

Vector4 MobileAdaptor::touchPosition(uint8_t index) {
    return s_Touches[index].pos;
}
