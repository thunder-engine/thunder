#include "adapters/desktopadaptor.h"

#ifdef _WIN32
    #include <Windows.h>
    #include <ShlObj.h>
#elif __GNUC__
    #include <sys/stat.h>
#endif
#include <GLFW/glfw3.h>

#include <timer.h>
#include <log.h>
#include <file.h>
#include <json.h>

#include <cstring>

#include "handlers/physfsfilehandler.h"
#include "handlers/fileloghandler.h"

#define CONFIG_NAME "config.json"

#define NONE -1
#define RELEASE 0
#define PRESS 1
#define REPEAT 2

namespace {
    const char *gScreenWidth("screen.width");
    const char *gScreenHeight("screen.height");
    const char *gScreenWindowed("screen.windowed");
    const char *gScreenVsync("screen.vsync");

    const char *gRhi(".rhi");
    const char *gCompany(".company");
    const char *gProject(".project");
};

Vector4 DesktopAdaptor::s_mousePosition = Vector4();
Vector4 DesktopAdaptor::s_oldMousePosition = Vector4();

float DesktopAdaptor::s_mouseScrollDelta = 0.0f;

int32_t DesktopAdaptor::s_width = 0;
int32_t DesktopAdaptor::s_height = 0;
bool DesktopAdaptor::s_windowed = false;
bool DesktopAdaptor::s_vSync = false;
bool DesktopAdaptor::s_appActive = true;

TString DesktopAdaptor::s_appConfig;
TString DesktopAdaptor::s_inputString;

std::unordered_map<int32_t, int32_t> DesktopAdaptor::s_keys;
std::unordered_map<int32_t, int32_t> DesktopAdaptor::s_mouseButtons;

DesktopAdaptor::DesktopAdaptor() :
        m_window(nullptr),
        m_monitor(nullptr),
        m_noOpenGL(false) {

    Log::setHandler(new DesktopLogHandler());
}

bool DesktopAdaptor::init() {
    glfwSetErrorCallback(errorCallback);

    if(!glfwInit()) {
        return false;
    }

    return true;
}

void DesktopAdaptor::update() {
    if(!m_noOpenGL) {
        glfwSwapBuffers(m_window);
    }

    s_inputString.clear();

    for(auto &it : s_keys) {
        switch(it.second) {
            case RELEASE: it.second = NONE; break;
            case PRESS: it.second = REPEAT; break;
            default: break;
        }
    }

    for(auto &it : s_mouseButtons) {
        switch(it.second) {
            case RELEASE: it.second = NONE; break;
            case PRESS: it.second = REPEAT; break;
            default: break;
        }
    }

    s_oldMousePosition = s_mousePosition;

    s_mouseScrollDelta = 0.0f;

    PlatformAdaptor::update();
}

void DesktopAdaptor::loop() {
    while(!glfwWindowShouldClose(m_window)) {
        Engine::update();

        glfwPollEvents();
    }
}

bool DesktopAdaptor::start() {
    PhysfsFileHandler *fileHandler = new PhysfsFileHandler;
    fileHandler->init("");
    fileHandler->searchPathAdd("base.pak");
    File::setHandler(fileHandler);

    if(!Engine::reloadBundle()) {
        Log(Log::ERR) << "Filed to load bundle";
        return false;
    }

    s_appConfig = Engine::locationAppConfig();
    static_cast<DesktopLogHandler *>(Log::handler())->setPath(s_appConfig);

#if _WIN32
    int32_t size = MultiByteToWideChar(CP_UTF8, 0, s_appConfig.data(), s_appConfig.size(), nullptr, 0);
    if(size) {
        std::wstring path;
        path.resize(size);
        MultiByteToWideChar(CP_UTF8, 0, s_appConfig.data(), s_appConfig.size(), path.data(), size);

        uint32_t start = 0;
        for(int32_t slash = 0; slash != -1; start = slash) {
            slash = path.find(L"/", start + 1);
            if(slash) {
                CreateDirectoryW(&path.substr(0, slash)[0], nullptr);
                DWORD error = GetLastError();
                if(error == ERROR_ALREADY_EXISTS || error == ERROR_ACCESS_DENIED) {
                    continue;
                }
                break;
            }
        }
    }
#else
    for(size_t i = 1; i < s_appConfig.size(); i++) {
        if(s_appConfig[i] == '/' || i == s_appConfig.size()) {
            int result = ::mkdir(s_appConfig.mid(0, i).data(), 0777);
            if(result != 0 && (errno == EEXIST || errno == EACCES)) {
                continue;
            }
        }
    }
#endif
    fileHandler->searchPathAdd(s_appConfig.data(), true);

    s_width = Engine::value(gScreenWidth, s_width).toInt();
    s_height = Engine::value(gScreenHeight, s_height).toInt();

    m_monitor = glfwGetPrimaryMonitor();
    if(m_monitor) {
        const GLFWvidmode *mode = glfwGetVideoMode(m_monitor);
        if(s_width <= 0) {
            s_width = mode->width;
        }
        if(s_height <= 0) {
            s_height = mode->height;
        }
    }

    if(!File::exists(CONFIG_NAME)) {
        Engine::syncValues();
    }

    File fp(CONFIG_NAME);
    if(fp.open(File::ReadOnly)) {
        ByteArray data(fp.readAll());

        Variant var = Json::load(TString(data));
        if(var.isValid()) {
            for(auto &it : var.toMap()) {
                Engine::setValue(it.first, it.second);
            }
        }
    }

    TString rhi = Engine::value(gRhi, "").toString();
    m_noOpenGL = (rhi == "RenderVK");

    if(m_noOpenGL) {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    } else {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    }

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    s_windowed = true;Engine::value(gScreenWindowed, s_windowed).toBool();
    s_vSync = Engine::value(gScreenVsync, s_vSync).toBool();

    m_window = glfwCreateWindow(s_width, s_height, Engine::value(gProject).toString().data(), (s_windowed) ? nullptr : m_monitor, nullptr);
    if(!m_window) {
        glfwTerminate();
        return false;
    }

    glfwSetWindowFocusCallback(m_window, windowFocusCallback);
    glfwSetCharCallback(m_window, charCallback);
    glfwSetKeyCallback(m_window, keyCallback);
    glfwSetMouseButtonCallback(m_window, buttonCallback);
    glfwSetScrollCallback(m_window, scrollCallback);
    glfwSetCursorPosCallback(m_window, cursorPositionCallback);

    if(!m_noOpenGL) {
        glfwSwapInterval(s_vSync);
        glfwMakeContextCurrent(m_window);
    }

    return true;
}

void DesktopAdaptor::destroy() {
    glfwTerminate();
}

bool DesktopAdaptor::isActive() const {
    return s_appActive;
}

bool DesktopAdaptor::key(Input::KeyCode code) const {
    return (m_window && glfwGetKey(m_window, code) == GLFW_PRESS);
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
    return (m_window && glfwGetMouseButton(m_window, button | 0x10000000) == GLFW_PRESS);
}

bool DesktopAdaptor::mousePressed(int button) const {
    return (s_mouseButtons[button | 0x10000000] == PRESS);
}

bool DesktopAdaptor::mouseReleased(int  button) const {
    return (s_mouseButtons[button | 0x10000000] == RELEASE);
}

void DesktopAdaptor::mouseLockCursor(bool lock) {
    glfwSetInputMode(m_window, GLFW_CURSOR, lock ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}

uint32_t DesktopAdaptor::screenWidth() const {
    return s_width;
}

uint32_t DesktopAdaptor::screenHeight() const {
    return s_height;
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

void DesktopAdaptor::toggleFullscreen(GLFWwindow *window) {
    if(s_windowed) {
        glfwSetWindowMonitor(window, NULL, 0, 30, s_width, s_height, GLFW_DONT_CARE);
        glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_TRUE); // Disable for frameless
    } else {
        GLFWmonitor *monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode *mode = glfwGetVideoMode(monitor);
        glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
    }

    s_windowed = !s_windowed;
}

void DesktopAdaptor::keyCallback(GLFWwindow *widnow, int code, int, int action, int mods) {
    s_keys[static_cast<Input::KeyCode>(code)] = action;

    if(code == GLFW_KEY_ENTER && action == GLFW_PRESS && (mods & GLFW_MOD_ALT)) {
        toggleFullscreen(widnow);
    }
}

void DesktopAdaptor::charCallback(GLFWwindow *, unsigned int codepoint) {
    s_inputString += TString::fromWc32(codepoint);
}

void DesktopAdaptor::buttonCallback(GLFWwindow *, int button, int action, int) {
    s_mouseButtons[button | 0x10000000] = action;
}

void DesktopAdaptor::scrollCallback(GLFWwindow *, double, double yoffset) {
    s_mouseScrollDelta = yoffset;
}

void DesktopAdaptor::cursorPositionCallback(GLFWwindow *, double xpos, double ypos) {
    s_mousePosition = Vector4(xpos, s_height - ypos, xpos / s_width, (s_height - ypos) / s_height);

    static bool first = true;
    if(first) {
        s_oldMousePosition = s_mousePosition;
        first = false;
    }
}

void DesktopAdaptor::errorCallback(int error, const char *description) {
    Log(Log::ERR) << "Desktop adaptor failed with code:" << error << description;
}

TString DesktopAdaptor::locationLocalDir() const {
    TString result;
#if _WIN32
    wchar_t path[MAX_PATH];
    if(SHGetSpecialFolderPathW(nullptr, path, CSIDL_LOCAL_APPDATA, FALSE)) {
        result = TString::fromWString(std::wstring(path));
        result.replace('\\', '/');
    }
#elif __APPLE__
    result = "~/Library/Preferences";
#else
    result += ::getenv("HOME");
    result += "/.config";
#endif

    TString organization(Engine::value(gCompany).toString());
    if(!organization.isEmpty()) {
        result += TString("/") + organization;
    }

    TString project(Engine::value(gProject).toString());
    if(!project.isEmpty()) {
        result += TString("/") + project;
    }

    return result;
}

void DesktopAdaptor::syncConfiguration(VariantMap &map) const {
    s_width = Engine::value(gScreenWidth, s_width).toInt();
    s_height = Engine::value(gScreenHeight, s_height).toInt();
    s_windowed = Engine::value(gScreenWindowed, s_windowed).toBool();
    s_vSync = Engine::value(gScreenVsync, s_vSync).toBool();

    if(m_window) {
        int32_t x, y;
        glfwGetWindowPos(m_window, &x, &y);
        glfwSetWindowMonitor(m_window, (s_windowed) ? nullptr : m_monitor, x, y, s_width, s_height, GLFW_DONT_CARE);

        //glfwSwapInterval(s_vSync);
    }

    map[gScreenWidth] = s_width;
    map[gScreenHeight] = s_height;
    map[gScreenWindowed] = s_windowed;
    map[gScreenVsync] = s_vSync;

    File fp(CONFIG_NAME);
    if(fp.open(File::WriteOnly)) {
        fp.write(Json::save(map, 0));
    }
}
