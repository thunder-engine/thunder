#include "adapters/desktopadaptor.h"
#if(_MSC_VER)
    #include <windows.h>
    #include <Shlobj.h>
#elif __APPLE__
    #include <CoreFoundation/CoreFoundation.h>
    #include <ApplicationServices/ApplicationServices.h>
#elif(__GNUC__)
    #include <dlfcn.h>
#endif
#include <GLFW/glfw3.h>

#include <log.h>
#include <file.h>

Vector3 DesktopAdaptor::s_MouseDelta     = Vector3();
Vector3 DesktopAdaptor::s_MousePosition  = Vector3();

static Engine *g_pEngine   = nullptr;

string wchar_to_utf8(const wstring &in) {
    string result;
    for(uint32_t i = 0; i < in.size(); i++) {
        uint32_t wc32   = in[i];
        if(wc32 < 0x007F) {
            result  += (char)wc32;
        }
        else if(wc32 < 0x07FF) {
            result  += (char)(0xC0 + (wc32 >> 6));
            result  += (char)(0x80 + (wc32 & 0x3F));
        }
        else if(wc32 < 0xFFFF) {
            result  += (char)(0xE0 + (wc32 >> 12));
            result  += (char)(0x80 + (wc32 >> 6 & 0x3F));
            result  += (char)(0x80 + (wc32 & 0x3F));
        }
        else {
            result  += (char)(0xF0 + (wc32 >> 18));
            result  += (char)(0x80 + (wc32 >> 12 & 0x3F));
            result  += (char)(0x80 + (wc32 >> 6 & 0x3F));
            result  += (char)(0x80 + (wc32 & 0x3F));
        }
    }
    return result;
}

bool DesktopAdaptor::init(Engine *engine) {
    g_pEngine   = engine;
    glfwSetErrorCallback(errorCallback);

    if(!glfwInit()) {
        return false;
    }

#ifdef __APPLE__
    /* We need to explicitly ask for a 3.2 context on OS X */
    glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint (GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint (GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_pMonitor  = glfwGetPrimaryMonitor();
    if(!m_pMonitor) {
        stop();
        return false;
    }

    return true;
}

void DesktopAdaptor::update() {
    glfwSwapBuffers(m_pWindow);
    glfwPollEvents();
    s_MousePosition += s_MouseDelta;

    m_MouseButtons  = 0;
    for(uint8_t i = 0; i < 8; i++) {
        if(glfwGetMouseButton(m_pWindow, i)) {
            m_MouseButtons  |= (1<<i);
        }
    }
}

bool DesktopAdaptor::start() {
    const GLFWvidmode *mode = glfwGetVideoMode(m_pMonitor);

    m_pWindow   = glfwCreateWindow(mode->width, mode->height, "Thunder Engine", nullptr, nullptr); // m_pMonitor
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

Vector3 DesktopAdaptor::mousePosition() {
    return s_MousePosition;
}

Vector3 DesktopAdaptor::mouseDelta() {
    return s_MouseDelta;
}

uint8_t DesktopAdaptor::mouseButtons() {
    return m_MouseButtons;
}

uint32_t DesktopAdaptor::screenWidth() {
    const GLFWvidmode *mode = glfwGetVideoMode(m_pMonitor);
    return mode->width;
}

uint32_t DesktopAdaptor::screenHeight() {
    const GLFWvidmode *mode = glfwGetVideoMode(m_pMonitor);
    return mode->height;
}

void DesktopAdaptor::setMousePosition(const Vector3 &position) {
    glfwSetCursorPos(m_pWindow, position.x, position.y);
}

uint16_t DesktopAdaptor::joystickCount() {
    uint16_t result  = 0;
    for(uint8_t i = 0; i <= GLFW_JOYSTICK_LAST; i++) {
        if(glfwJoystickPresent(GLFW_JOYSTICK_1 + i)) {
            result++;
        }
    }
    return result;
}

uint16_t DesktopAdaptor::joystickButtons(uint8_t index) {
    int count;
    const unsigned char *axes = glfwGetJoystickButtons(index, &count);
    uint16_t result = 0;
    for(int i = 0; i < count; i++) {
        if(axes[i] == GLFW_PRESS && i < 16) {
            result |= (1<<i);
        }
    }
    return result;
}

Vector4 DesktopAdaptor::joystickThumbs(uint8_t index) {
    int count;
    const float *axes = glfwGetJoystickAxes(index, &count);
    if(count >= 4) {
        return Vector4(axes[0], axes[1], axes[2], axes[3]);
    }
    return Vector4();
}

Vector2 DesktopAdaptor::joystickTriggers(uint8_t index) {
    int count;
    const float* axes = glfwGetJoystickAxes(index, &count);
    if(count >= 6) {
        return Vector2(axes[4], axes[5]);
    }
    return Vector2();
}

void *DesktopAdaptor::pluginLoad(const char *name) {
#if(_MSC_VER)
    return (void*)LoadLibrary((LPCWSTR)name);
#elif(__GNUC__)
    return dlopen(name, RTLD_NOW);
#endif
}

bool DesktopAdaptor::pluginUnload(void *plugin) {
#if(_MSC_VER)
    return FreeLibrary((HINSTANCE)plugin);
#elif(__GNUC__)
    return dlclose(plugin);
#endif
}

void *DesktopAdaptor::pluginAddress(void *plugin, const string &name) {
#if(_MSC_VER)
    return (void*)GetProcAddress((HINSTANCE)plugin, name.c_str());
#elif(__GNUC__)
    return dlsym(plugin, name.c_str());
#endif
}

void DesktopAdaptor::scrollCallback(GLFWwindow *, double, double yoffset) {
    s_MouseDelta    = Vector3(s_MouseDelta.x, s_MouseDelta.y, yoffset);
}

void DesktopAdaptor::cursorPositionCallback(GLFWwindow *, double xpos, double ypos) {
    s_MouseDelta    = Vector3(xpos, ypos, s_MouseDelta.z);
}

void DesktopAdaptor::errorCallback(int error, const char *description) {
    Log(Log::ERR) << "Desktop adaptor failed with code:" << error << description;
}

string DesktopAdaptor::locationLocalDir() {
    string result;
#if _WIN32
    wchar_t path[MAX_PATH];
    if(SHGetSpecialFolderPath(0, path, CSIDL_LOCAL_APPDATA, FALSE)) {
        result  = wchar_to_utf8(wstring(path));
        std::replace(result.begin(), result.end(), '\\', '/');
    }
#endif
    return result;
}
