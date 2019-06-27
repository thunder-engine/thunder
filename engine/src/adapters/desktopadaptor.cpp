#include "adapters/desktopadaptor.h"
#ifdef _WIN32
    #include <Windows.h>
    #include <ShlObj.h>
#elif __APPLE__
    #include <CoreFoundation/CoreFoundation.h>
#endif
#ifdef __GNUC__
    #include <dlfcn.h>
#endif
#include <GLFW/glfw3.h>

#include <log.h>
#include <file.h>
#include <utils.h>

#include <mutex>
#include <string.h>

Vector4 DesktopAdaptor::s_MousePosition     = Vector4();
Vector4 DesktopAdaptor::s_OldMousePosition  = Vector4();
Vector2 DesktopAdaptor::s_Screen            = Vector2();

static Engine *g_pEngine = nullptr;
static IFile *g_pFile = nullptr;

static string gAppConfig;

class DesktopHandler : public ILogHandler {
protected:
    void            setRecord       (Log::LogTypes, const char *record) {
        unique_lock<mutex> locker(m_Mutex);
        FILE *fp   = fopen((gAppConfig + "/log.txt").c_str(), "a");
        if(fp) {
            fwrite(record, strlen(record), 1, fp);
            fwrite("\n", 1, 1, fp);
            fclose(fp);
        }
    }

    mutex           m_Mutex;
};

DesktopAdaptor::DesktopAdaptor(Engine *engine) :
        m_pWindow(nullptr),
        m_pMonitor(nullptr),
        m_MouseButtons(0) {
    g_pEngine   = engine;

}

bool DesktopAdaptor::init() {
    gAppConfig  = g_pEngine->locationAppConfig();
    g_pFile = g_pEngine->file();

    glfwSetErrorCallback(errorCallback);

    if(!glfwInit()) {
        return false;
    }

    glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint (GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint (GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_pMonitor  = glfwGetPrimaryMonitor();
    if(!m_pMonitor) {
        stop();
        return false;
    }

    const GLFWvidmode *mode = glfwGetVideoMode(m_pMonitor);
    s_Screen = Vector2(mode->width, mode->height);

    return true;
}

void DesktopAdaptor::update() {
    glfwSwapBuffers(m_pWindow);
    glfwPollEvents();

    m_MouseButtons  = 0;
    for(uint8_t i = 0; i < 8; i++) {
        if(glfwGetMouseButton(m_pWindow, i)) {
            m_MouseButtons  |= (1<<i);
        }
    }
}

bool DesktopAdaptor::start() {
    g_pFile->fsearchPathAdd(g_pEngine->locationAppConfig().c_str(), true);
    g_pFile->_mkdir(g_pEngine->locationAppConfig().c_str());
    Log::overrideHandler(new DesktopHandler());

    g_pFile->fsearchPathAdd((g_pEngine->locationAppDir() + "/base.pak").c_str());

    const GLFWvidmode *mode = glfwGetVideoMode(m_pMonitor);
    m_pWindow = glfwCreateWindow(mode->width, mode->height, "Thunder Engine", nullptr, nullptr); // m_pMonitor
    if(!m_pWindow) {
        stop();
        return false;
    }

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

Vector4 DesktopAdaptor::mousePosition() {
    return s_MousePosition;
}

Vector4 DesktopAdaptor::mouseDelta() {
    return s_MousePosition - s_OldMousePosition;
}

uint32_t DesktopAdaptor::mouseButtons() {
    return m_MouseButtons;
}

uint32_t DesktopAdaptor::screenWidth() {
    return s_Screen.x;
}

uint32_t DesktopAdaptor::screenHeight() {
    return s_Screen.y;
}

void DesktopAdaptor::setMousePosition(int32_t x, int32_t y) {
    glfwSetCursorPos(m_pWindow, x, y);
}

uint32_t DesktopAdaptor::joystickCount() {
    uint16_t result  = 0;
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

void DesktopAdaptor::scrollCallback(GLFWwindow *, double, double yoffset) {
    A_UNUSED(yoffset)
}

void DesktopAdaptor::cursorPositionCallback(GLFWwindow *, double xpos, double ypos) {
    s_OldMousePosition  = s_MousePosition;
    s_MousePosition = Vector4(xpos, ypos, xpos / s_Screen.x, ypos / s_Screen.y);
}

void DesktopAdaptor::errorCallback(int error, const char *description) {
    Log(Log::ERR) << "Desktop adaptor failed with code:" << error << description;
}

string DesktopAdaptor::locationLocalDir() {
    string result;
#if _WIN32
    wchar_t path[MAX_PATH];
    if(SHGetSpecialFolderPathW(nullptr, path, CSIDL_LOCAL_APPDATA, FALSE)) {
        result  = Utils::wstringToUtf8(wstring(path));
        replace(result.begin(), result.end(), '\\', '/');
    }
#elif __APPLE__
    result = "~/Library/Preferences";
#endif
    return result;
}
