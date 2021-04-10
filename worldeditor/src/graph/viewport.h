#ifndef VIEWPORT_H
#define VIEWPORT_H

#include <QOpenGLWidget>

class CameraCtrl;
class Scene;

class Viewport : public QOpenGLWidget {
    Q_OBJECT
public:
    Viewport(QWidget *parent = 0);

    void setController(CameraCtrl *ctrl);
    CameraCtrl *controller() const { return m_pController; }

    void setScene(Scene *scene);
    Scene *scene() { return m_pScene; }

signals:
    void inited();

    void drop(QDropEvent *);
    void dragEnter(QDragEnterEvent *);
    void dragMove(QDragMoveEvent *);
    void dragLeave(QDragLeaveEvent *);

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;

    void dragEnterEvent(QDragEnterEvent *) override;
    void dragMoveEvent(QDragMoveEvent *) override;
    void dragLeaveEvent(QDragLeaveEvent *) override;
    void dropEvent(QDropEvent *) override;

    void mouseMoveEvent(QMouseEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;

    void wheelEvent(QWheelEvent *) override;

    void keyPressEvent(QKeyEvent *) override;
    void keyReleaseEvent(QKeyEvent *) override;

private:
    void findCamera();

    CameraCtrl *m_pController;

    Scene *m_pScene;

};

#endif // VIEWPORT_H
