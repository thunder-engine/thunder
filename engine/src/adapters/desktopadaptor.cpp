#include "adapters/desktopadaptor.h"
#ifdef _WIN32
    #include <Windows.h>
    #include <ShlObj.h>
#elif __APPLE__
    #include <CoreFoundation/CoreFoundation.h>
#endif
#ifdef __GNUC__
    #include <dlfcn.h>
    #include <sys/stat.h>
#endif
#include <GLFW/glfw3.h>

#include <log.h>
#include <file.h>
#include <utils.h>
#include <json.h>

#include <mutex>
#include <algorithm>
#include <string>
#include <cstring>

#define CONFIG_NAME "config.json"

namespace {
    const char *SCREEN_WIDTH("screen.width");
    const char *SCREEN_HEIGHT("screen.height");
    const char *SCREEN_WINDOWED("screen.windowed");
    const char *SCREEN_VSYNC("screen.vsync");
};

#define NONE -1
#define RELEASE 0
#define PRESS 1
#define REPEAT 2

Vector4 DesktopAdaptor::s_MousePosition = Vector4();
Vector4 DesktopAdaptor::s_OldMousePosition = Vector4();
int32_t DesktopAdaptor::s_Width = 0;
int32_t DesktopAdaptor::s_Height = 0;
bool DesktopAdaptor::s_Windowed = false;
bool DesktopAdaptor::s_vSync = false;
bool DesktopAdaptor::s_mouseLocked = false;

static string gAppConfig;

static unordered_map<int32_t, int32_t> s_Keys;
static unordered_map<int32_t, int32_t> s_MouseButtons;

static string s_inputString;

class DesktopHandler : public LogHandler {
protected:
    void setRecord(Log::LogTypes, const char *record) {
        unique_lock<mutex> locker(m_Mutex);
        FILE *fp = fopen((gAppConfig + "/log.txt").c_str(), "a");
        if(fp) {
            fwrite(record, strlen(record), 1, fp);
            fwrite("\n", 1, 1, fp);
            fclose(fp);
        }
    }

    mutex m_Mutex;
};

DesktopAdaptor::DesktopAdaptor(const string &rhi) :
        m_pWindow(nullptr),
        m_pMonitor(nullptr),
        m_rhi(rhi) {

    Log::overrideHandler(new DesktopHandler());
}

bool DesktopAdaptor::init() {
    glfwSetErrorCallback(errorCallback);

    if(!glfwInit()) {
        return false;
    }

    if(m_rhi == "RenderVK") {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    } else {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    }

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    return true;
}

void DesktopAdaptor::update() {
    glfwSwapBuffers(m_pWindow);

    s_inputString.clear();

    for(auto &it : s_Keys) {
        switch(it.second) {
            case RELEASE: it.second = NONE; break;
            case PRESS: it.second = REPEAT; break;
            default: break;
        }
    }

    for(auto &it : s_MouseButtons) {
        switch(it.second) {
            case RELEASE: it.second = NONE; break;
            case PRESS: it.second = REPEAT; break;
            default: break;
        }
    }

    PlatformAdaptor::update();

    if(s_mouseLocked) {
        glfwSetCursorPos(m_pWindow, screenWidth() / 2 , screenHeight() / 2);
    }

    glfwPollEvents();
}

bool DesktopAdaptor::start() {
    File *file = Engine::file();

    file->fsearchPathAdd((Engine::locationAppDir() + "/base.pak").c_str());

    if(Engine::reloadBundle() == false) {
        Log(Log::ERR) << "Filed to load bundle";
    }

    gAppConfig = Engine::locationAppConfig();

#if _WIN32
    int32_t size = MultiByteToWideChar(CP_UTF8, 0, gAppConfig.c_str(), gAppConfig.size(), nullptr, 0);
    if(size) {
        wstring path;
        path.resize(size);
        MultiByteToWideChar(CP_UTF8, 0, gAppConfig.c_str(), gAppConfig.size(), &path[0], size);

        uint32_t start = 0;
        for(int32_t slash=0; slash != -1; start = slash) {
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
    for(size_t i = 1; i <= gAppConfig.size(); i++) {
        if(gAppConfig[i] == '/' || i == gAppConfig.size()) {
            int result = ::mkdir(gAppConfig.substr(0, i).c_str(), 0777);
            if(result != 0 && (errno == EEXIST || errno == EACCES)) {
                continue;
            }
        }
    }
#endif
    file->fsearchPathAdd(gAppConfig.c_str(), true);

    s_Width = Engine::value(SCREEN_WIDTH, s_Width).toInt();
    s_Height = Engine::value(SCREEN_HEIGHT, s_Height).toInt();

    m_pMonitor = glfwGetPrimaryMonitor();
    if(m_pMonitor) {
        const GLFWvidmode *mode = glfwGetVideoMode(m_pMonitor);
        if(s_Width <= 0) {
            s_Width = mode->width;
        }
        if(s_Height <= 0) {
            s_Height = mode->height;
        }
    }

    if(!file->exists(CONFIG_NAME)) {
        Engine::syncValues();
    }

    _FILE *fp = file->fopen(CONFIG_NAME, "r");
    if(fp) {
        ByteArray data;
        data.resize(file->fsize(fp));
        file->fread(&data[0], data.size(), 1, fp);
        file->fclose(fp);

        Variant var = Json::load(string(data.begin(), data.end()));
        if(var.isValid()) {
            for(auto &it : var.toMap()) {
                Engine::setValue(it.first, it.second);
            }
        }
    }

    s_Windowed = Engine::value(SCREEN_WINDOWED, s_Windowed).toBool();
    s_vSync = Engine::value(SCREEN_VSYNC, s_vSync).toBool();

    m_pWindow = glfwCreateWindow(s_Width, s_Height, Engine::applicationName().c_str(), (s_Windowed) ? nullptr : m_pMonitor, nullptr);
    if(!m_pWindow) {
        stop();
        return false;
    }

    glfwSwapInterval(s_vSync);

    glfwSetCharCallback(m_pWindow, charCallback);
    glfwSetKeyCallback(m_pWindow, keyCallback);
    glfwSetMouseButtonCallback(m_pWindow, buttonCallback);
    glfwSetScrollCallback(m_pWindow, scrollCallback);
    glfwSetCursorPosCallback(m_pWindow, cursorPositionCallback);

    glfwMakeContextCurrent(m_pWindow);

    return true;
}

void DesktopAdaptor::stop() {
    glfwTerminate();
}

void DesktopAdaptor::destroy() {

}

bool DesktopAdaptor::isValid() {
    return !glfwWindowShouldClose(m_pWindow);
}

bool DesktopAdaptor::key(Input::KeyCode code) const {
    return (glfwGetKey(m_pWindow, code) == GLFW_PRESS);
}

bool DesktopAdaptor::keyPressed(Input::KeyCode code) const {
    return (s_Keys[code] == PRESS);
}

bool DesktopAdaptor::keyReleased(Input::KeyCode code) const {
    return (s_Keys[code] == RELEASE);
}

string DesktopAdaptor::inputString() const {
    return s_inputString;
}

Vector4 DesktopAdaptor::mousePosition() const {
    return s_MousePosition;
}

Vector4 DesktopAdaptor::mouseDelta() const {
    return s_MousePosition - s_OldMousePosition;
}

bool DesktopAdaptor::mouseButton(int button) const {
    return (glfwGetMouseButton(m_pWindow, button | 0x10000000) == GLFW_PRESS);
}

bool DesktopAdaptor::mousePressed(int button) const {
    return (s_MouseButtons[button | 0x10000000] == PRESS);
}

bool DesktopAdaptor::mouseReleased(int  button) const {
    return (s_MouseButtons[button | 0x10000000] == RELEASE);
}

void DesktopAdaptor::mouseLockCursor(bool lock) {
    s_mouseLocked = lock;
    glfwSetInputMode(m_pWindow, GLFW_CURSOR, s_mouseLocked ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);
}

uint32_t DesktopAdaptor::screenWidth() const {
    return s_Width;
}

uint32_t DesktopAdaptor::screenHeight() const {
    return s_Height;
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

void *DesktopAdaptor::pluginLoad(const char *name) {
#ifdef WIN32
    return static_cast<void*>(LoadLibraryW(reinterpret_cast<LPCWSTR>(name)));
#elif(__GNUC__)
    return dlopen(name, RTLD_NOW);
#endif
}

bool DesktopAdaptor::pluginUnload(void *plugin) {
#ifdef WIN32
    return FreeLibrary(reinterpret_cast<HINSTANCE>(plugin));
#elif(__GNUC__)
    return dlclose(plugin);
#endif
}

void *DesktopAdaptor::pluginAddress(void *plugin, const string &name) {
#ifdef WIN32
    return (void*)GetProcAddress(reinterpret_cast<HINSTANCE>(plugin), name.c_str());
#elif(__GNUC__)
    return dlsym(plugin, name.c_str());
#endif
}

void DesktopAdaptor::keyCallback(GLFWwindow *, int code, int, int action, int) {
    s_Keys[static_cast<Input::KeyCode>(code)] = action;
}

void DesktopAdaptor::charCallback(GLFWwindow *, unsigned int codepoint) {
    s_inputString += Utils::wc32ToUtf8(codepoint);
}

void DesktopAdaptor::buttonCallback(GLFWwindow *, int button, int action, int) {
    s_MouseButtons[button] = action;
}

void DesktopAdaptor::scrollCallback(GLFWwindow *, double, double yoffset) {
    A_UNUSED(yoffset);
}

void DesktopAdaptor::cursorPositionCallback(GLFWwindow *, double xpos, double ypos) {
    s_OldMousePosition = s_MousePosition;
    s_MousePosition = Vector4(xpos, s_Height - ypos, xpos / s_Width, (s_Height - ypos) / s_Height);
}

void DesktopAdaptor::errorCallback(int error, const char *description) {
    Log(Log::ERR) << "Desktop adaptor failed with code:" << error << description;
}

string DesktopAdaptor::locationLocalDir() const {
    string result;
#if _WIN32
    wchar_t path[MAX_PATH];
    if(SHGetSpecialFolderPathW(nullptr, path, CSIDL_LOCAL_APPDATA, FALSE)) {
        result = Utils::wstringToUtf8(wstring(path));
        replace(result.begin(), result.end(), '\\', '/');
    }
#elif __APPLE__
    result = "~/Library/Preferences";
#else
    result += ::getenv("HOME");
    result += "/.config";
#endif
    return result;
}

void DesktopAdaptor::syncConfiguration(VariantMap &map) const {
    s_Width = Engine::value(SCREEN_WIDTH, s_Width).toInt();
    s_Height = Engine::value(SCREEN_HEIGHT, s_Height).toInt();
    s_Windowed = Engine::value(SCREEN_WINDOWED, s_Windowed).toBool();
    s_vSync = Engine::value(SCREEN_VSYNC, s_vSync).toBool();

    if(m_pWindow) {
        int32_t x, y;
        glfwGetWindowPos(m_pWindow, &x, &y);
        glfwSetWindowMonitor(m_pWindow, (s_Windowed) ? nullptr : m_pMonitor, x, y, s_Width, s_Height, GLFW_DONT_CARE);

        glfwSwapInterval(s_vSync);
    }

    map[SCREEN_WIDTH] = s_Width;
    map[SCREEN_HEIGHT] = s_Height;
    map[SCREEN_WINDOWED] = s_Windowed;
    map[SCREEN_VSYNC] = s_vSync;

    File *file = Engine::file();

    _FILE *fp = file->fopen(CONFIG_NAME, "w");
    if(fp) {
        string data = Json::save(map, 0);
        file->fwrite(&data[0], data.size(), 1, fp);
        file->fclose(fp);
    }
}
