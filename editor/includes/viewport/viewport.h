/*
    This file is part of Thunder Engine.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
#ifndef VIEWPORT_H
#define VIEWPORT_H

#include <QWidget>

#include <editor.h>

class CameraController;

class PipelineContext;
class PipelineTask;

class Texture;
class Camera;
class World;

class Outline;
class GizmoRender;
class GridRender;
class DebugRender;

class QMenu;

class EDITOR_EXPORT Viewport : public QWidget {
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

    bool isFocused();

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

    QAction *addAction(QMenu *menu, const TString &name);

protected slots:
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

    PipelineContext *m_pipelineContext;
    QWindow *m_rhiWindow;

    Texture *m_color;

    bool m_focusedView;
    bool m_gameView;
    bool m_gamePaused;
    bool m_liveUpdate;
    bool m_frameInProgress;
    bool m_screenInProgress;

};

#endif // VIEWPORT_H
