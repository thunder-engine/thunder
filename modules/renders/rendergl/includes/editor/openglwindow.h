#ifndef OPENGLWINDOW_H
#define OPENGLWINDOW_H

#include <QOpenGLWindow>

class OpenGLWindow : public QOpenGLWindow {
    Q_OBJECT

public:
    OpenGLWindow() {
        startTimer(16);
    }

signals:
    void draw();

protected:
    void paintGL() override {
        QOpenGLWindow::paintGL();

        emit draw();
    }

    void timerEvent(QTimerEvent *) override {
        update();
    }
};

#endif // OPENGLWINDOW_H
