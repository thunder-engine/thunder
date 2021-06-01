#ifndef OPENGLWINDOW_H
#define OPENGLWINDOW_H

#include <QOpenGLWindow>

class OpenGLWindow : public QOpenGLWindow {
    Q_OBJECT

signals:
    void draw();

protected:
    void paintGL() override {
        QOpenGLWindow::paintGL();

        emit draw();
        update();
    }
};

#endif // OPENGLWINDOW_H
