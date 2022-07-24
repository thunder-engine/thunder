#ifndef VIEWPORT_H
#define VIEWPORT_H

#include <QWidget>

#include <engine.h>

class CameraCtrl;
class Scene;

class ENGINE_EXPORT Viewport : public QWidget {
    Q_OBJECT
public:
    Viewport(QWidget *parent = 0);

    void init();

    void setController(CameraCtrl *ctrl);
    void setSceneGraph(SceneGraph *scene);

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
    CameraCtrl *m_controller;

    SceneGraph *m_sceneGraph;

    QWindow *m_rhiWindow;

};

#endif // VIEWPORT_H
