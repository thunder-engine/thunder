#ifndef VIEWPORT_H
#define VIEWPORT_H

#include <QWidget>

#include <engine.h>

class CameraCtrl;

class PipelineContext;
class Outline;
class GizmoRender;
class GridRender;
class DebugRender;
class PipelinePass;

class QMenu;

class ENGINE_EXPORT Viewport : public QWidget {
    Q_OBJECT
public:
    Viewport(QWidget *parent = 0);

    void init();

    void setController(CameraCtrl *ctrl);
    virtual void setWorld(World *scene);

    QImage grabFramebuffer() { return QImage(); }

    void createMenu(QMenu *menu);

    PipelineContext *pipelineContext() const;

    float gridCell();

    void setGridEnabled(bool enabled);
    void setGizmoEnabled(bool enabled);
    void setOutlineEnabled(bool enabled);

    void addPass(PipelinePass *pass);

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

    void onBufferChanged(bool checked);
    void onPostEffectChanged(bool checked);

protected:
    CameraCtrl *m_controller;

    World *m_sceneGraph;

    Outline *m_outlinePass;
    GizmoRender *m_gizmoRender;
    GridRender *m_gridRender;
    DebugRender *m_debugRender;

    RenderSystem *m_renderSystem;
    QWindow *m_rhiWindow;

    QMenu *m_postMenu;
    QMenu *m_lightMenu;
    QMenu *m_bufferMenu;

};

#endif // VIEWPORT_H
