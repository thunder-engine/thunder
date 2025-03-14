#ifndef OPENGLWINDOW_H
#define OPENGLWINDOW_H

#include <QOpenGLWindow>

#include <editor/viewport/viewport.h>

class OpenGLWindow : public QOpenGLWindow {
    Q_OBJECT

public:
    OpenGLWindow(Viewport *viewport) :
            m_viewport(viewport) {
        connect(this, SIGNAL(frameSwapped()), this, SLOT(update()));
    }

protected:
    void paintGL() override {
        QOpenGLWindow::paintGL();

        m_viewport->onDraw();
    }

private:
    Viewport *m_viewport;

};

#endif // OPENGLWINDOW_H
