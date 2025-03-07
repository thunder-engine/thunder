#include "editor/rhiwrapper.h"

#include "editor/openglwindow.h"

#include <QOpenGLContext>
#include <QOffscreenSurface>

QWindow *createWindow(Viewport *viewport) {
    return new OpenGLWindow(viewport);
}

void makeCurrent() {
    static QOffscreenSurface *surface = nullptr;
    static QOpenGLContext *context = nullptr;

    if(surface == nullptr) {
        surface = new QOffscreenSurface();
        surface->create();

        context = new QOpenGLContext();
        context->setShareContext(QOpenGLContext::globalShareContext());
        context->setFormat(surface->requestedFormat());
        context->create();

    }
    context->makeCurrent(surface);
}
