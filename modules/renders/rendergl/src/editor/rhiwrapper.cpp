#include "editor/rhiwrapper.h"

#include "editor/openglwindow.h"

QWindow *createWindow() {
    return new OpenGLWindow();
}
