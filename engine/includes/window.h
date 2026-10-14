#ifndef WINDOW_H
#define WINDOW_H

#include <cstdint>

#include "engine.h"

struct GLFWcursor;
struct GLFWmonitor;
struct GLFWwindow;

class PipelineContext;
class FileDropTarget;
class NativeMenuBar;

class ENGINE_EXPORT Window : public Object {
    A_OBJECT(Window, Object, General)

public:
    enum Flags : uint32_t {
        None = 0,
        Windowed = (1 << 0),
        VSync = (1 << 1),
        Resizable = (1 << 2),
        Maximized = (1 << 3)
    };

    Window();
    ~Window() override;

    bool create(int32_t width, int32_t height, const char *title, uint32_t flags);
    void destroy();

    GLFWwindow *handle() const;
    static Window *fromHandle(GLFWwindow *handle);

    bool shouldClose() const;
    int exec();
    bool key(int code) const;
    bool mouseButton(int button) const;

    void swapBuffers() const;
    void setCursorLocked(bool lock) const;
    void setCursor(GLFWcursor *cursor) const;
    void setFileDropTarget(FileDropTarget *target);
    // Window does not own the model; it must outlive Window. Returns false only when
    // native attachment is supported but fails. Unsupported platforms retain the model
    // for shortcut handling and use the UiKit MenuBar as fallback.
    bool setMenuBar(NativeMenuBar *menuBar);
    void toggleFullscreen();
    void setSize(int32_t width, int32_t height) const;
    void setFlags(uint32_t flags) const;

    int32_t width() const;
    int32_t height() const;
    bool isWindowed() const;
    bool isVSync() const;
    bool usesNoOpenGL() const;

protected:
    virtual void beforeRender();

protected:
    PipelineContext *m_context;
    World *m_world;

private:
    using KeyCallback = void (*)(GLFWwindow *, int, int, int, int);

    static void sizeCallback(GLFWwindow *handle, int width, int height);
    static void fileDropCallback(GLFWwindow *handle, int count, const char **paths);
    static void menuKeyCallback(GLFWwindow *handle, int key, int scancode, int action, int mods);
    void restoreMenuKeyCallback();

    GLFWwindow *m_handle;
    GLFWmonitor *m_monitor;

    mutable int32_t m_width;
    mutable int32_t m_height;

    mutable bool m_windowed;
    mutable bool m_vSync;

    bool m_noOpenGL;
    FileDropTarget *m_fileDropTarget;
    NativeMenuBar *m_menuBar;
    bool m_menuKeyCallbackInstalled;
    KeyCallback m_previousKeyCallback;
};

#endif // WINDOW_H
