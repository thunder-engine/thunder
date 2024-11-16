#ifndef OPENGLWINDOW_H
#define OPENGLWINDOW_H

#include <QOpenGLWindow>

class OpenGLWindow : public QOpenGLWindow {
    Q_OBJECT

public:
    OpenGLWindow() {
        connect(this, SIGNAL(frameSwapped()), this, SLOT(update()));
    }

signals:
    void draw();

protected:
    void paintGL() override {
        QOpenGLWindow::paintGL();

        emit draw();
    }
};

#endif // OPENGLWINDOW_H
