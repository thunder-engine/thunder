#ifndef VIEWPORT_H
#define VIEWPORT_H

#include <QWidget>

#include <engine.h>

class CameraController;

class PipelineContext;
class Outline;
class GizmoRender;
class GridRender;
class DebugRender;
class PipelineTask;

class QMenu;

class ENGINE_EXPORT Viewport : public QWidget {
    Q_OBJECT
public:
    Viewport(QWidget *parent = 0);

    void init();

    CameraController *controllder();
    void setController(CameraController *ctrl);
    virtual void setWorld(World *world);

    QImage grabFramebuffer() { return QImage(); }

    void createMenu(QMenu *menu);

    PipelineContext *pipelineContext() const;

    float gridCell();

    bool isGamePaused() const;
    void setGamePaused(bool pause);

    bool isLiveUpdate() const;
    void setLiveUpdate(bool update);

    void setGameView(bool enabled);
    void setSceneView(bool enabled);

    void setGridEnabled(bool enabled);
    void setGizmoEnabled(bool enabled);
    void setOutlineEnabled(bool enabled);

    void addRenderTask(PipelineTask *task);

    QWindow *rhiWindow() { return m_rhiWindow; }

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

    void resizeEvent(QResizeEvent *event) override;

    void fillTasksMenu(QMenu *menu);

protected slots:
    void onApplySettings();

    virtual void onDraw();

    void onBufferMenu();

    void onBufferChanged(bool checked);
    void onPostEffectChanged(bool checked);

protected:
    QPoint m_savedMousePos;

    CameraController *m_controller;

    World *m_world;

    Outline *m_outlinePass;
    GizmoRender *m_gizmoRender;
    GridRender *m_gridRender;
    DebugRender *m_debugRender;

    RenderSystem *m_renderSystem;
    QWindow *m_rhiWindow;

    QMenu *m_tasksMenu;
    QMenu *m_bufferMenu;

    bool m_gameView;
    bool m_gamePaused;
    bool m_liveUpdate;

};

#endif // VIEWPORT_H
