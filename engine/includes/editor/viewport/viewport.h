#ifndef VIEWPORT_H
#define VIEWPORT_H

#include <QOpenGLWidget>

#include <global.h>

class CameraCtrl;
class Scene;

class NEXT_LIBRARY_EXPORT Viewport : public QWidget {
    Q_OBJECT
public:
    Viewport(QWidget *parent = 0);

    void setController(CameraCtrl *ctrl);
    CameraCtrl *controller() const { return m_pController; }

    void setScene(Scene *scene);

    QImage grabFramebuffer() { return QImage(); }

public slots:
    void onCursorSet(const QCursor &cursor);
    void onCursorUnset();

signals:
    void drop(QDropEvent *);
    void dragEnter(QDragEnterEvent *);
    void dragMove(QDragMoveEvent *);
    void dragLeave(QDragLeaveEvent *);

protected:
    bool eventFilter(QObject *object, QEvent *event) override;

private slots:
    void onDraw();

private:
    CameraCtrl *m_pController;

    Scene *m_pScene;

    QWindow *m_pRHIWindow;

};

#endif // VIEWPORT_H
