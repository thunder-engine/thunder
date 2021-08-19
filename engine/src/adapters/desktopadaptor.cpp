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

#define SCREEN_WIDTH "screen.width"
#define SCREEN_HEIGHT "screen.height"
#define SCREEN_WINDOWED "screen.windowed"

#define NONE -1
#define RELEASE 0
#define PRESS 1
#define REPEAT 2

Vector4 DesktopAdaptor::s_MousePosition  = Vector4();
Vector4 DesktopAdaptor::s_OldMousePosition = Vector4();
int32_t DesktopAdaptor::s_Width  = 0;
int32_t DesktopAdaptor::s_Height = 0;
bool DesktopAdaptor::s_Windowed  = false;

static Engine *g_pEngine = nullptr;
static File *g_pFile = nullptr;

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

DesktopAdaptor::DesktopAdaptor(Engine *engine) :
        m_pWindow(nullptr),
        m_pMonitor(nullptr) {
    g_pEngine = engine;

}

bool DesktopAdaptor::init() {
    glfwSetErrorCallback(errorCallback);

    if(!glfwInit()) {
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

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

    glfwPollEvents();
}

bool DesktopAdaptor::start() {
    Log::overrideHandler(new DesktopHandler());

    g_pFile->fsearchPathAdd((g_pEngine->locationAppDir() + "/base.pak").c_str());

    if(Engine::reloadBundle() == false) {
        Log(Log::ERR) << "Filed to load bundle";
    }

    gAppConfig = g_pEngine->locationAppConfig();
    g_pFile = g_pEngine->file();

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
    ::mkdir(gAppConfig.c_str(), 0777);
#endif
    g_pFile->fsearchPathAdd(gAppConfig.c_str(), true);

    _FILE *fp = g_pFile->fopen(CONFIG_NAME, "r");
    if(fp) {
        ByteArray data;
        data.resize(g_pFile->fsize(fp));
        g_pFile->fread(&data[0], data.size(), 1, fp);
        g_pFile->fclose(fp);

        Variant var = Json::load(string(data.begin(), data.end()));
        if(var.isValid()) {
            for(auto it : var.toMap()) {
                Engine::setValue(it.first, it.second);
            }
        }
    }

    m_pMonitor = glfwGetPrimaryMonitor();
    if(m_pMonitor) {
        const GLFWvidmode *mode = glfwGetVideoMode(m_pMonitor);
        s_Width = mode->width;
        s_Height = mode->height;
    }
    s_Width = Engine::value(SCREEN_WIDTH, s_Width).toInt();
    s_Height = Engine::value(SCREEN_HEIGHT, s_Height).toInt();
    s_Windowed = Engine::value(SCREEN_WINDOWED, s_Windowed).toBool();

    m_pWindow = glfwCreateWindow(s_Width, s_Height, g_pEngine->applicationName().c_str(), (s_Windowed) ? nullptr : m_pMonitor, nullptr);
    if(!m_pWindow) {
        stop();
        return false;
    }
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

bool DesktopAdaptor::key(Input::KeyCode code) {
    return (glfwGetKey(m_pWindow, code) == GLFW_PRESS);
}

bool DesktopAdaptor::keyPressed(Input::KeyCode code) {
    return (s_Keys[code] == PRESS);
}

bool DesktopAdaptor::keyReleased(Input::KeyCode code) {
    return (s_Keys[code] == RELEASE);
}

string DesktopAdaptor::inputString() {
    return s_inputString;
}

Vector4 DesktopAdaptor::mousePosition() {
    return s_MousePosition;
}

Vector4 DesktopAdaptor::mouseDelta() {
    return s_MousePosition - s_OldMousePosition;
}

bool DesktopAdaptor::mouseButton(Input::MouseButton button) {
    return (glfwGetMouseButton(m_pWindow, button) == GLFW_PRESS);
}

bool DesktopAdaptor::mousePressed(Input::MouseButton button) {
    return (s_MouseButtons[button] == PRESS);
}

bool DesktopAdaptor::mouseReleased(Input::MouseButton button) {
    return (s_MouseButtons[button] == RELEASE);
}

uint32_t DesktopAdaptor::screenWidth() {
    return s_Width;
}

uint32_t DesktopAdaptor::screenHeight() {
    return s_Height;
}

void DesktopAdaptor::setMousePosition(int32_t x, int32_t y) {
    glfwSetCursorPos(m_pWindow, x, y);
}

uint32_t DesktopAdaptor::joystickCount() {
    uint16_t result = 0;
    for(uint8_t i = 0; i <= GLFW_JOYSTICK_LAST; i++) {
        if(glfwJoystickPresent(GLFW_JOYSTICK_1 + i)) {
            result++;
        }
    }
    return result;
}

uint32_t DesktopAdaptor::joystickButtons(uint32_t index) {
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

Vector4 DesktopAdaptor::joystickThumbs(uint32_t index) {
    int count;
    const float *axes = glfwGetJoystickAxes(index, &count);
    if(count >= 4) {
        return Vector4(axes[0], axes[1], axes[2], axes[3]);
    }
    return Vector4();
}

Vector2 DesktopAdaptor::joystickTriggers(uint32_t index) {
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
    s_MouseButtons[static_cast<Input::MouseButton>(button)] = action;
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

string DesktopAdaptor::locationLocalDir() {
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
    result = "~/.config";
#endif
    return result;
}

void DesktopAdaptor::syncConfiguration(VariantMap &map) const {
    s_Width = Engine::value(SCREEN_WIDTH, s_Width).toInt();
    s_Height = Engine::value(SCREEN_HEIGHT, s_Height).toInt();
    s_Windowed = Engine::value(SCREEN_WINDOWED, s_Windowed).toBool();

    int32_t x, y;
    glfwGetWindowPos(m_pWindow, &x, &y);
    glfwSetWindowMonitor(m_pWindow, (s_Windowed) ? nullptr : m_pMonitor, x, y, s_Width, s_Height, GLFW_DONT_CARE);

    _FILE *fp = g_pFile->fopen(CONFIG_NAME, "w");
    if(fp) {
        string data = Json::save(map, 0);
        g_pFile->fwrite(&data[0], data.size(), 1, fp);
        g_pFile->fclose(fp);
    }
}
