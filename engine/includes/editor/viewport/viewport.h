#ifndef VIEWPORT_H
#define VIEWPORT_H

#include <QWidget>

#include <engine.h>

class CameraController;

class PipelineContext;
class PipelineTask;

class Texture;
class Camera;

class Outline;
class GizmoRender;
class GridRender;
class DebugRender;

class QMenu;

class ENGINE_EXPORT Viewport : public QWidget {
    Q_OBJECT
public:
    Viewport(QWidget *parent = 0);

    void init();
    virtual void onDraw();

    CameraController *controller();
    void setController(CameraController *ctrl);
    virtual void setWorld(World *world);

    void setCamera(Camera *camera);

    void createMenu(QMenu *menu);

    PipelineContext *pipelineContext() const;

    void grabScreen();

    int gridCell();

    bool isGamePaused() const;
    void setGamePaused(bool pause);

    bool isLiveUpdate() const;
    void setLiveUpdate(bool update);

    void setGameView(bool enabled);

    void setGridEnabled(bool enabled);
    void setGizmoEnabled(bool enabled);
    void setOutlineEnabled(bool enabled);
    void setGuiEnabled(bool enabled);

    void showCube(bool enabled);
    void showGizmos(bool enabled);

    void addRenderTask(PipelineTask *task);

    static void readPixels(void *object);

    QWindow *rhiWindow() { return m_rhiWindow; }

    bool isFocused() const;

public slots:
    void onInProgressFlag(bool flag);

    void onCursorSet(const QCursor &cursor);
    void onCursorUnset();

signals:
    void drop(QDropEvent *);
    void dragEnter(QDragEnterEvent *);
    void dragMove(QDragMoveEvent *);
    void dragLeave(QDragLeaveEvent *);

    void screenshot(QImage);

protected:
    bool event(QEvent *event) override;

    bool eventFilter(QObject *object, QEvent *event) override;

    bool processEvent(QEvent *event);

    void resizeEvent(QResizeEvent *event) override;

    void fillTasksMenu(QMenu *menu);

protected slots:
    void onBufferMenu();

    void onBufferChanged(bool checked);
    void onPostEffectChanged(bool checked);

protected:
    QPoint m_savedMousePos;

    CameraController *m_controller;

    World *m_world;
    Camera *m_camera;

    PipelineTask *m_guiLayer;

    Outline *m_outlinePass;
    GizmoRender *m_gizmoRender;
    GridRender *m_gridRender;
    DebugRender *m_debugRender;

    RenderSystem *m_renderSystem;
    QWindow *m_rhiWindow;

    QMenu *m_tasksMenu;
    QMenu *m_bufferMenu;

    Texture *m_color;

    bool m_gameView;
    bool m_gamePaused;
    bool m_liveUpdate;
    bool m_frameInProgress;
    bool m_screenInProgress;

};

#endif // VIEWPORT_H
