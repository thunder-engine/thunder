#include "adapters/desktopadaptor.h"

#include <GLFW/glfw3.h>

#include <timer.h>
#include <log.h>
#include <file.h>
#include <json.h>
#include <standardpaths.h>

#include <filesystem>
#include "handlers/physfsfilehandler.h"
#include "systems/resourcesystem.h"

#define NONE -1
#define RELEASE 0
#define PRESS 1
#define REPEAT 2
#define DOUBLE 3

namespace {
    const char *gConfigName("config.json");

    const char *gScreenWidth("s.width");
    const char *gScreenHeight("s.height");
    const char *gScreenWindowed("s.windowed");
    const char *gScreenVsync("s.vsync");

    const float DOUBLE_CLICK_TIME = 0.3f;
    const float DOUBLE_CLICK_DIST = 5.0f;

};

Vector4 DesktopAdaptor::s_mousePosition = Vector4();
Vector4 DesktopAdaptor::s_oldMousePosition = Vector4();

float DesktopAdaptor::s_mouseScrollDelta = 0.0f;

bool DesktopAdaptor::s_appActive = true;

TString DesktopAdaptor::s_inputString;

std::unordered_map<int32_t, int32_t> DesktopAdaptor::s_keys;
std::unordered_map<int32_t, int32_t> DesktopAdaptor::s_mouseButtons;
std::unordered_map<int32_t, GLFWcursor *> DesktopAdaptor::s_mouseCursors;

double DesktopAdaptor::s_lastClickTime   = -1.0;
int    DesktopAdaptor::s_lastClickButton = -1;
double DesktopAdaptor::s_lastClickX      = 0.0;
double DesktopAdaptor::s_lastClickY      = 0.0;

DesktopAdaptor::DesktopAdaptor() = default;

bool DesktopAdaptor::init() {
    glfwSetErrorCallback(errorCallback);

    if(!glfwInit()) {
        return false;
    }

    return true;
}

void DesktopAdaptor::update() {
    if(m_window) {
        m_window->swapBuffers();
    }

    s_inputString.clear();

    for(auto &it : s_keys) {
        switch(it.second) {
        case RELEASE: it.second = NONE; break;
        case PRESS:   it.second = REPEAT; break;
        default: break;
        }
    }

    for(auto &it : s_mouseButtons) {
        switch(it.second) {
        case RELEASE: it.second = NONE; break;
        case PRESS:   it.second = REPEAT; break;
        case DOUBLE:  it.second = NONE; break;
        default: break;
        }
    }

    s_oldMousePosition = s_mousePosition;
    s_mouseScrollDelta = 0.0f;

    PlatformAdaptor::update();
}

void DesktopAdaptor::loop() {
    while(m_window && !m_window->shouldClose()) {
        Engine::update(Engine::world());

        Engine::platformAdaptor()->update();

        glfwPollEvents();
    }
}

bool DesktopAdaptor::start() {
    PhysfsFileHandler *fileHandler = new PhysfsFileHandler("");
    File::setHandler(fileHandler);

    if(!Engine::resourceSystem()->loadBundle("base.pak")) {
        aError() << "Filed to load bundle";
        return false;
    }

    fileHandler->mount(locationLocalDir().data(), true);

    if(!File::exists(gConfigName)) {
        Engine::syncValues();
    }

    File fp(gConfigName);
    if(fp.open(File::Read)) {
        Variant var = Json::load(TString(fp.readAll()));
        if(var.isValid()) {
            for(auto &it : var.toMap()) {
                Engine::setValue(it.first, it.second);
            }
        }
    }

    const int32_t width = Engine::value(gScreenWidth, 0).toInt();
    const int32_t height = Engine::value(gScreenHeight, 0).toInt();
    uint32_t windowFlags = Window::None;
    if(Engine::value(gScreenWindowed, false).toBool()) {
        windowFlags |= Window::Windowed;
    }
    if(Engine::value(gScreenVsync, false).toBool()) {
        windowFlags |= Window::VSync;
    }

    m_window = new Window;
    if(!m_window->create(width, height, Engine::applicationName().data(), windowFlags)) {
        glfwTerminate();
        return false;
    }

    return true;
}

void DesktopAdaptor::destroy() {
    if(m_window) {
        m_window->destroy();
    }
    glfwTerminate();
}

bool DesktopAdaptor::isActive() const {
    return s_appActive;
}

bool DesktopAdaptor::key(Input::KeyCode code) const {
    if(m_window) {
        return m_window->key(code);
    }
    return false;
}

bool DesktopAdaptor::keyPressed(Input::KeyCode code) const {
    return (s_keys[code] == PRESS);
}

bool DesktopAdaptor::keyReleased(Input::KeyCode code) const {
    return (s_keys[code] == RELEASE);
}

TString DesktopAdaptor::inputString() const {
    return s_inputString;
}

Vector4 DesktopAdaptor::mousePosition() const {
    return s_mousePosition;
}

Vector4 DesktopAdaptor::mouseDelta() const {
    return s_mousePosition - s_oldMousePosition;
}

float DesktopAdaptor::mouseScrollDelta() const {
    return s_mouseScrollDelta;
}

bool DesktopAdaptor::mouseButton(int button) const {
    if(m_window) {
        return m_window->mouseButton(button & ~0x10000000);
    }
    return false;
}

bool DesktopAdaptor::mousePressed(int button) const {
    const int key = button | 0x10000000;
    auto it = s_mouseButtons.find(key);
    return (it != s_mouseButtons.end() && (it->second == PRESS));
}

bool DesktopAdaptor::mouseReleased(int button) const {
    const int key = button | 0x10000000;
    auto it = s_mouseButtons.find(key);
    return (it != s_mouseButtons.end() && it->second == RELEASE);
}

bool DesktopAdaptor::mouseButtonDoubleClick(int button) const {
    const int key = button | 0x10000000;
    auto it = s_mouseButtons.find(key);
    return (it != s_mouseButtons.end() && it->second == DOUBLE);
}

void DesktopAdaptor::mouseLockCursor(bool lock) {
    if(m_window) {
        m_window->setCursorLocked(lock);
    }
}

void DesktopAdaptor::mouseSetCursor(Input::CursorShape shape) {
    GLFWcursor *cursor = nullptr;

    if(shape != Input::CURSOR_ARROW) {
        auto it = s_mouseCursors.find(shape);
        if(it == s_mouseCursors.end()) {
            int glfwShape = 0;
            switch(shape) {
                case Input::CURSOR_ARROW: glfwShape = GLFW_ARROW_CURSOR; break;
                case Input::CURSOR_CROSS: glfwShape = GLFW_CROSSHAIR_CURSOR; break;
                case Input::CURSOR_IBEAM: glfwShape = GLFW_IBEAM_CURSOR; break;
                case Input::CURSOR_HAND: glfwShape = GLFW_POINTING_HAND_CURSOR; break;
                case Input::CURSOR_HORSIZE: glfwShape = GLFW_HRESIZE_CURSOR; break;
                case Input::CURSOR_VERSIZE: glfwShape = GLFW_VRESIZE_CURSOR; break;
                case Input::CURSOR_FDIAGSIZE: glfwShape = GLFW_RESIZE_NWSE_CURSOR; break;
                case Input::CURSOR_BDIAGSIZE: glfwShape = GLFW_RESIZE_NESW_CURSOR; break;
                case Input::CURSOR_ALLSIZE: glfwShape = GLFW_RESIZE_ALL_CURSOR; break;
                default: break;
            }

            if(glfwShape != 0) {
                cursor = glfwCreateStandardCursor(glfwShape);
            }
        } else {
            cursor = it->second;
        }
    }

    if(m_window) {
        m_window->setCursor(cursor);
    }
}

uint32_t DesktopAdaptor::screenWidth() const {
    if(m_window) {
        return m_window->width();
    }
    return 0;
}

uint32_t DesktopAdaptor::screenHeight() const {
    if(m_window) {
        return m_window->height();
    }
    return 0;
}

uint32_t DesktopAdaptor::joystickCount() const {
    uint16_t result = 0;
    for(uint8_t i = 0; i <= GLFW_JOYSTICK_LAST; i++) {
        if(glfwJoystickPresent(GLFW_JOYSTICK_1 + i)) {
            result++;
        }
    }
    return result;
}

uint32_t DesktopAdaptor::joystickButtons(int index) const {
    int count;
    const unsigned char *axes = glfwGetJoystickButtons(index, &count);
    uint16_t result = 0;
    for(int i = 0; i < count; i++) {
        if(i < 16 && axes[i] == GLFW_PRESS) {
            result |= (1<<i);
        }
    }
    return result;
}

Vector4 DesktopAdaptor::joystickThumbs(int index) const {
    int count;
    const float *axes = glfwGetJoystickAxes(index, &count);
    if(count >= 4) {
        return Vector4(axes[0], axes[1], axes[2], axes[3]);
    }
    return Vector4();
}

Vector2 DesktopAdaptor::joystickTriggers(int index) const {
    int count;
    const float* axes = glfwGetJoystickAxes(index, &count);
    if(count >= 6) {
        return Vector2(axes[4], axes[5]);
    }
    return Vector2();
}

void DesktopAdaptor::windowFocusCallback(GLFWwindow *, int focused) {
    s_appActive = focused;
}

void DesktopAdaptor::keyCallback(GLFWwindow *window, int code, int, int action, int mods) {
    s_keys[static_cast<Input::KeyCode>(code)] = action;

    if(code == GLFW_KEY_ENTER && action == GLFW_PRESS && (mods & GLFW_MOD_ALT)) {
        Window::fromHandle(window)->toggleFullscreen();
    }
}

void DesktopAdaptor::charCallback(GLFWwindow *, unsigned int codepoint) {
    s_inputString += TString::fromWc32(codepoint);
}

void DesktopAdaptor::buttonCallback(GLFWwindow *window, int button, int action, int) {
    const int key = button | 0x10000000;

    if(action == RELEASE) {
        s_mouseButtons[key] = RELEASE;
        return;
    }

    if(action != PRESS) {
        return;
    }

    const double now = glfwGetTime();
    double x = 0.0, y = 0.0;
    glfwGetCursorPos(window, &x, &y);

    const bool sameButton = (button == s_lastClickButton);
    const bool inTime = (now - s_lastClickTime) < DOUBLE_CLICK_TIME;
    const bool inPlace = std::abs(x - s_lastClickX) < DOUBLE_CLICK_DIST && std::abs(y - s_lastClickY) < DOUBLE_CLICK_DIST;

    if(sameButton && inTime && inPlace) {
        s_mouseButtons[key] = DOUBLE;
        s_lastClickTime = -1.0;
        s_lastClickButton = -1;
    } else {
        s_mouseButtons[key] = PRESS;
        s_lastClickTime = now;
        s_lastClickButton = button;
        s_lastClickX  = x;
        s_lastClickY = y;
    }
}

void DesktopAdaptor::scrollCallback(GLFWwindow *, double, double yoffset) {
    s_mouseScrollDelta = yoffset;
}

void DesktopAdaptor::cursorPositionCallback(GLFWwindow *handle, double xpos, double ypos) {
    Window *window = Window::fromHandle(handle);
    const int32_t width = window->width();
    const int32_t height = window->height();
    s_mousePosition = Vector4(xpos, height - ypos, xpos / width, (height - ypos) / height);

    static bool first = true;
    if(first) {
        s_oldMousePosition = s_mousePosition;
        first = false;
    }
}

void DesktopAdaptor::errorCallback(int error, const char *description) {
    aError() << "Desktop adaptor failed with code:" << error << description;
}

Window *DesktopAdaptor::window() {
    return m_window;
}

void DesktopAdaptor::setWindow(Window *window) {
    m_window = window;
}

TString DesktopAdaptor::locationLocalDir() const {
    TString result = StandardPaths::writableLocation(StandardPaths::ConfigLocation);

    TString organization(Engine::organizationName());
    if(!organization.isEmpty()) {
        result += TString("/") + organization;
    }

    TString project(Engine::applicationName());
    if(!project.isEmpty()) {
        result += TString("/") + project;
    }

    if(!std::filesystem::exists(result.toStdString())) {
        std::filesystem::create_directories(result.toStdString());
    }

    return result;
}

void DesktopAdaptor::syncConfiguration(VariantMap &map) const {
    if(m_window == nullptr) {
        return;
    }
    const int32_t width = Engine::value(gScreenWidth, m_window->width()).toInt();
    const int32_t height = Engine::value(gScreenHeight, m_window->height()).toInt();
    const bool windowed = Engine::value(gScreenWindowed, m_window->isWindowed()).toBool();
    const bool vSync = Engine::value(gScreenVsync, m_window->isVSync()).toBool();

    uint32_t windowFlags = Window::None;
    if(windowed) {
        windowFlags |= Window::Windowed;
    }
    if(vSync) {
        windowFlags |= Window::VSync;
    }

    m_window->setFlags(windowFlags);
    m_window->setSize(width, height);

    map[gScreenWidth] = width;
    map[gScreenHeight] = height;
    map[gScreenWindowed] = windowed;
    map[gScreenVsync] = vSync;

    File fp(gConfigName);
    if(fp.open(File::Write)) {
        fp.write(Json::save(map, 0));
    }
}
