#include "../includes/window.h"

#include <adapters/desktopadaptor.h>

#include <GLFW/glfw3.h>

#include <pipelinecontext.h>
#include <systems/rendersystem.h>
#include <systems/resourcesystem.h>
#include <os/filedroptarget.h>
#include <os/menubar.h>
#include <log.h>

#include <string>

#if defined(_WIN32)
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#elif defined(__APPLE__)
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3native.h>
#endif

/*! 
    \class Window
    \brief Represents an engine-managed native desktop window.
    \inmodule Engine

    Window owns a GLFW window and exposes its lifecycle, display mode, input polling,
    and presentation operations. The renderer backend is selected from the Engine
    settings when the window is created.
*/

/*! 
    \enum Window::Flags
    \brief Options that configure a window at creation time.

    \value None Use the default fullscreen mode without vertical synchronization.
    \value Windowed Create the window in decorated windowed mode.
    \value VSync Synchronize buffer swaps with the display refresh rate.
    \value Resizable Allow resizing the window and enable the native maximize control.
    \value Maximized Create or set the window in maximized mode; implies Resizable.
*/

/*! 
    Constructs a window object without creating a native window.
*/
Window::Window() :
        m_context(nullptr),
        m_world(nullptr),
        m_handle(nullptr),
        m_monitor(nullptr),
        m_width(0),
        m_height(0),
        m_windowed(false),
        m_vSync(false),
        m_noOpenGL(false),
        m_fileDropTarget(nullptr),
        m_menuBar(nullptr),
        m_menuKeyCallbackInstalled(false),
        m_previousKeyCallback(nullptr) {
}

/*! 
    Destroys the native window if it is still owned by this object.
*/
Window::~Window() {
    destroy();
}

/*! 
    Creates the native window with the requested dimensions, \a title, and display \a flags.
    Non-positive dimensions use the primary monitor's current resolution. The renderer
    backend is selected from the Engine \c .rhi setting.
*/
bool Window::create(int32_t width, int32_t height, const char *title, uint32_t flags) {
    m_monitor = glfwGetPrimaryMonitor();
    if(m_monitor) {
        const GLFWvidmode *mode = glfwGetVideoMode(m_monitor);
        if(width <= 0) {
            width = mode->width;
        }
        if(height <= 0) {
            height = mode->height;
        }
    }

    if(m_world == nullptr) {
        m_world = Engine::world();
    }

    m_width = width;
    m_height = height;
    m_windowed = (flags & Windowed) != 0;
    m_vSync = (flags & VSync) != 0;
    m_noOpenGL = Engine::value(".rhi", "").toString() == "RenderVK";

    if(m_noOpenGL) {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    } else {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    }

    const bool maximized = (flags & Window::Flags::Maximized) != 0;
    glfwWindowHint(GLFW_RESIZABLE, (flags & Window::Flags::Resizable) || maximized ? GLFW_TRUE : GLFW_FALSE);
    glfwWindowHint(GLFW_MAXIMIZED, maximized ? GLFW_TRUE : GLFW_FALSE);

    m_handle = glfwCreateWindow(m_width, m_height, title, m_windowed ? nullptr : m_monitor, nullptr);
    if(!m_handle) {
        return false;
    }

    glfwSetWindowUserPointer(m_handle, this);
    glfwSetWindowSizeCallback(m_handle, sizeCallback);
    glfwSetDropCallback(m_handle, fileDropCallback);
    glfwSetWindowFocusCallback(m_handle, DesktopAdaptor::windowFocusCallback);
    glfwSetCharCallback(m_handle, DesktopAdaptor::charCallback);
    glfwSetKeyCallback(m_handle, DesktopAdaptor::keyCallback);
    glfwSetMouseButtonCallback(m_handle, DesktopAdaptor::buttonCallback);
    glfwSetScrollCallback(m_handle, DesktopAdaptor::scrollCallback);
    glfwSetCursorPosCallback(m_handle, DesktopAdaptor::cursorPositionCallback);

    double xpos = 0.0;
    double ypos = 0.0;
    glfwGetCursorPos(m_handle, &xpos, &ypos);
    DesktopAdaptor::cursorPositionCallback(m_handle, xpos, ypos);

    if(m_menuBar) {
        if(!setMenuBar(m_menuBar)) {
            aWarning() << "Failed to attach the native menu bar.";
        }
    }

    if(!m_noOpenGL) {
        glfwMakeContextCurrent(m_handle);
        glfwSwapInterval(m_vSync);
    }

    Engine::platformAdaptor()->setWindow(this);

    return true;
}

void Window::sizeCallback(GLFWwindow *handle, int width, int height) {
    if(Window *window = fromHandle(handle)) {
        window->m_width = width;
        window->m_height = height;
    }
}

void Window::fileDropCallback(GLFWwindow *handle, int count, const char **paths) {
    Window *window = fromHandle(handle);
    if(!window || !window->m_fileDropTarget || count <= 0 || !paths) {
        return;
    }
    StringList files;
    for(int i = 0; i < count; ++i) {
        if(paths[i]) {
            files.push_back(TString(std::string(paths[i])));
        }
    }
    window->m_fileDropTarget->deliverFiles(files);
}

void Window::menuKeyCallback(GLFWwindow *handle, int key, int scancode, int action, int mods) {
    Window *window = fromHandle(handle);
    KeyCallback previousCallback = window ? window->m_previousKeyCallback : nullptr;
    bool menuIsAttached = false;
    if(window && window->m_menuBar) {
#if defined(_WIN32)
        menuIsAttached = window->m_menuBar->isAttachedTo(
                    reinterpret_cast<void *>(glfwGetWin32Window(handle)));
#elif defined(__APPLE__)
        menuIsAttached = window->m_menuBar->isAttachedTo(
                    reinterpret_cast<void *>(glfwGetCocoaWindow(handle)));
#endif
    }
    if(menuIsAttached && action == GLFW_PRESS) {
        window->m_menuBar->triggerShortcut(key, mods);
    }
    if(previousCallback && previousCallback != menuKeyCallback) {
        previousCallback(handle, key, scancode, action, mods);
    }
}

void Window::restoreMenuKeyCallback() {
    if(m_menuKeyCallbackInstalled && m_handle) {
        KeyCallback currentCallback = glfwSetKeyCallback(m_handle, m_previousKeyCallback);
        if(currentCallback != menuKeyCallback) {
            glfwSetKeyCallback(m_handle, currentCallback);
        }
    }
    m_menuKeyCallbackInstalled = false;
    m_previousKeyCallback = nullptr;
}

/*! 
    Destroys the native window while keeping this Window object alive.
*/
void Window::destroy() {
    if(m_handle) {
        if(m_menuBar && m_menuBar->isAttached()) {
            m_menuBar->detach();
        }
        glfwDestroyWindow(m_handle);
        m_handle = nullptr;
        m_menuKeyCallbackInstalled = false;
        m_previousKeyCallback = nullptr;
    }
}

/*! 
    Returns the underlying GLFW handle, or null if no native window exists.
*/
GLFWwindow *Window::handle() const {
    return m_handle;
}

/*! 
    Returns the Window associated with a GLFW handle, or null if it has no Window owner.
*/
Window *Window::fromHandle(GLFWwindow *handle) {
    return static_cast<Window *>(glfwGetWindowUserPointer(handle));
}

/*! 
    Returns whether the native window has been asked to close. An uncreated window is closed.
*/
bool Window::shouldClose() const {
    return !m_handle || glfwWindowShouldClose(m_handle);
}

void Window::beforeRender() {

}
/*! 
    Runs the native window event loop until the window is closed.
*/
int Window::exec() {
    if(!m_handle) {
        return 1;
    }

    while(!shouldClose()) {
        glfwPollEvents();

        Engine::instance().processEvents();

        if(ResourceSystem *resourceSystem = Engine::resourceSystem()) {
            resourceSystem->processEvents();
        }

        // Draws all offscreen images
        beforeRender();

        if(RenderSystem *renderSystem = Engine::renderSystem()) {
            renderSystem->setPipelineContext(m_context);

            if(m_context) {
                int framebufferWidth, framebufferHeight;
                glfwGetFramebufferSize(m_handle, &framebufferWidth, &framebufferHeight);
                m_context->resize(framebufferWidth, framebufferHeight);
            }

            renderSystem->setActiveWorld(m_world);
            renderSystem->processEvents();
        }

        Engine::platformAdaptor()->update();
    }

    return 0;
}

/*! 
    Returns whether the specified keyboard key \a code is currently pressed in this window.
*/
bool Window::key(int code) const {
    return m_handle && glfwGetKey(m_handle, code) == GLFW_PRESS;
}

/*! 
    Returns whether the specified mouse \a button is currently pressed in this window.
*/
bool Window::mouseButton(int button) const {
    return m_handle && glfwGetMouseButton(m_handle, button) == GLFW_PRESS;
}

/*! 
    Presents the rendered frame when this window uses an OpenGL context.
*/
void Window::swapBuffers() const {
    if(m_handle && !m_noOpenGL) {
        glfwSwapBuffers(m_handle);
    }
}

/*! 
    Enables or disables cursor capture for this window.
*/
void Window::setCursorLocked(bool lock) const {
    if(m_handle) {
        glfwSetInputMode(m_handle, GLFW_CURSOR, lock ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    }
}

/*! 
    Sets the native cursor displayed over this window.
*/
void Window::setCursor(GLFWcursor *cursor) const {
    if(m_handle) {
        glfwSetCursor(m_handle, cursor);
    }
}

void Window::setFileDropTarget(FileDropTarget *target) {
    m_fileDropTarget = target;
    if(m_handle) {
        glfwSetDropCallback(m_handle, fileDropCallback);
    }
}

bool Window::setMenuBar(NativeMenuBar *menuBar) {
    if(m_menuBar == menuBar && m_menuBar && m_menuBar->isAttached()) {
        return true;
    }
    if(m_menuBar && m_menuBar != menuBar && m_menuBar->isAttached()) {
        m_menuBar->detach();
    }
    if(m_menuKeyCallbackInstalled &&
        (m_menuBar != menuBar || (m_menuBar && !m_menuBar->isAttached()))) {
        restoreMenuKeyCallback();
    }
    m_menuBar = menuBar;
    if(!m_menuBar) {
        restoreMenuKeyCallback();
        return true;
    }
    if(!m_handle) {
        return true;
    }
#if defined(_WIN32)
    const bool attached = m_menuBar->attach(reinterpret_cast<void *>(glfwGetWin32Window(m_handle)));
#elif defined(__APPLE__)
    const bool attached = m_menuBar->attach(reinterpret_cast<void *>(glfwGetCocoaWindow(m_handle)));
#else
    return true;
#endif
#if defined(_WIN32) || defined(__APPLE__)
    if(attached) {
        m_previousKeyCallback = glfwSetKeyCallback(m_handle, menuKeyCallback);
        if(m_previousKeyCallback == menuKeyCallback) {
            m_previousKeyCallback = nullptr;
        }
        m_menuKeyCallbackInstalled = true;
    }
    return attached;
#endif
}

/*! 
    Toggles this window between windowed and primary-monitor fullscreen modes.
*/
void Window::toggleFullscreen() {
    if(!m_handle) {
        return;
    }

    if(m_windowed) {
        glfwSetWindowMonitor(m_handle, nullptr, 0, 30, m_width, m_height, GLFW_DONT_CARE);
        glfwSetWindowAttrib(m_handle, GLFW_DECORATED, GLFW_TRUE);
    } else if(m_monitor) {
        const GLFWvidmode *mode = glfwGetVideoMode(m_monitor);
        glfwSetWindowMonitor(m_handle, m_monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
    }

    m_windowed = !m_windowed;
}

/*! 
    Resizes the native window or changes the fullscreen resolution.
*/
void Window::setSize(int32_t width, int32_t height) const {
    m_width = width;
    m_height = height;

    if(m_handle) {
        if(m_windowed) {
            glfwSetWindowSize(m_handle, m_width, m_height);
        } else if(m_monitor) {
            glfwSetWindowMonitor(m_handle, m_monitor, 0, 0, m_width, m_height, GLFW_DONT_CARE);
        }
    }
}

/*! 
    Applies window mode and presentation \a flags independently of the window size.
*/
void Window::setFlags(uint32_t flags) const {
    const bool windowed = (flags & Windowed) != 0;
    const bool vSync = (flags & VSync) != 0;
    const bool maximized = (flags & Window::Flags::Maximized) != 0;
    const bool windowedChanged = m_windowed != windowed;
    const bool vSyncChanged = m_vSync != vSync;

    m_windowed = windowed;
    m_vSync = vSync;

    if(m_handle && windowedChanged) {
        int32_t x, y;
        glfwGetWindowPos(m_handle, &x, &y);
        glfwSetWindowMonitor(m_handle, m_windowed ? nullptr : m_monitor, x, y, m_width, m_height, GLFW_DONT_CARE);
    }

    if(m_handle) {
        glfwSetWindowAttrib(m_handle, GLFW_RESIZABLE, (flags & Window::Flags::Resizable) || maximized ? GLFW_TRUE : GLFW_FALSE);
        if(maximized) {
            glfwMaximizeWindow(m_handle);
        } else {
            glfwRestoreWindow(m_handle);
        }
    }

    if(m_handle && vSyncChanged && !m_noOpenGL) {
        GLFWwindow *currentContext = glfwGetCurrentContext();
        glfwMakeContextCurrent(m_handle);
        glfwSwapInterval(m_vSync);
        glfwMakeContextCurrent(currentContext);
    }
}

/*! Returns the configured window width in pixels. */
int32_t Window::width() const {
    return m_width;
}

/*! Returns the configured window height in pixels. */
int32_t Window::height() const {
    return m_height;
}

/*! Returns whether the window is in windowed mode. */
bool Window::isWindowed() const {
    return m_windowed;
}

/*! Returns whether vertical synchronization is enabled. */
bool Window::isVSync() const {
    return m_vSync;
}

/*! whether the window was created without an OpenGL context. */
bool Window::usesNoOpenGL() const {
    return m_noOpenGL;
}
