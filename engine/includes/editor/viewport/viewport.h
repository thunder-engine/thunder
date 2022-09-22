#ifndef VIEWPORT_H
#define VIEWPORT_H

#include <QWidget>

#include <engine.h>

class CameraCtrl;

class PipelineContext;
class Outline;
class GizmoRender;

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

    PipelineContext *pipelineContext() const;

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

protected slots:
    void onApplySettings();

    virtual void onDraw();

    void onBufferMenu();

    void onBufferChanged();
    void onPostEffectChanged(bool checked);

protected:
    CameraCtrl *m_controller;

    SceneGraph *m_sceneGraph;

    Outline *m_outlinePass;
    GizmoRender *m_gizmoRender;

    RenderSystem *m_renderSystem;
    QWindow *m_rhiWindow;

    QMenu *m_postMenu;
    QMenu *m_lightMenu;
    QMenu *m_bufferMenu;

};

#endif // VIEWPORT_H
