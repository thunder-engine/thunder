#include "adapters/mobileadaptor.h"

#include <glfm.h>

#include <log.h>
#include <file.h>
#include <android/log.h>

#include "engine.h"

#include "androidfile.h"

class AndroidHandler : public ILogHandler {
protected:
    void setRecord (Log::LogTypes, const char *record) {
        __android_log_write(ANDROID_LOG_DEBUG, "ThunderEngine", record);
    }
};

static GLFMDisplay *gDisplay = nullptr;
static Engine *g_pEngine = nullptr;

Vector2 MobileAdaptor::s_Screen = Vector2();

struct Touch {
    uint32_t    phase;
    Vector2     pos;
};

typedef map<int, Touch> TouchMap;

static TouchMap s_Touches;

void onFrame(GLFMDisplay *, const double) {
    if(g_pEngine) {
        g_pEngine->update();
    }
}

void onResize(GLFMDisplay *, int width, int height) {
    MobileAdaptor::s_Screen = Vector2(width, height);
}

bool onTouch(GLFMDisplay *, int touch, GLFMTouchPhase phase, double x, double y) {
    if(phase < GLFMTouchPhaseEnded) {
        Touch t;
        t.phase = phase;
        t.pos = Vector2(x, y);
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

    Log::overrideHandler(new AndroidHandler());

    glfmSetDisplayConfig(gDisplay,
                         GLFMRenderingAPIOpenGLES31,
                         GLFMColorFormatRGBA8888,
                         GLFMDepthFormatNone,
                         GLFMStencilFormatNone,
                         GLFMMultisampleNone);

    glfmSetMainLoopFunc(gDisplay, onFrame);
    glfmSetSurfaceCreatedFunc(gDisplay, onResize);
    glfmSetSurfaceResizedFunc(gDisplay, onResize);

    glfmSetTouchFunc(gDisplay, onTouch);

    char *path = "";
    Engine *engine = new Engine(new AndroidFile(), 1, &path);
    thunderMain(engine);
}

MobileAdaptor::MobileAdaptor(Engine *engine) {
    g_pEngine   = engine;
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

Vector2 MobileAdaptor::touchPosition(uint8_t index) {
    return s_Touches[index].pos;
}
