#ifndef VIEWPORT_H
#define VIEWPORT_H

#include <QWidget>

#include <engine.h>

class CameraCtrl;
class Scene;
class Camera;

class MaterialInstance;
class PipelineContext;

class QMenu;

class ENGINE_EXPORT Viewport : public QWidget {
    Q_OBJECT
public:
    Viewport(QWidget *parent = 0);

    void init();

    void setController(CameraCtrl *ctrl);
    void setSceneGraph(SceneGraph *scene);

    QImage grabFramebuffer() { return QImage(); }

    void createMenu(QMenu *menu);

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

    void fillEffectMenu(QMenu *menu, uint32_t layers);

private slots:
    void onApplySettings();

    void onDraw();

    void onBufferMenu();

    void onBufferChanged();
    void onPostEffectChanged(bool checked);

private:
    CameraCtrl *m_controller;

    SceneGraph *m_sceneGraph;

    PipelineContext *m_pipelineContext;

    QWindow *m_rhiWindow;

    QMenu *m_postMenu;
    QMenu *m_lightMenu;
    QMenu *m_bufferMenu;

};

#endif // VIEWPORT_H
