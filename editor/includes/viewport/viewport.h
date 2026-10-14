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

#include <components/canvas.h>

#include <editor.h>

#include <functional>

class Action;
class PipelineContext;
class PipelineTask;
class CameraController;

class Camera;
class World;

class Outline;
class GizmoRender;
class GridRender;
class DebugRender;

class Menu;

class EDITOR_EXPORT Viewport : public Canvas {
    A_OBJECT(Viewport, Canvas, Components/UI)

    A_NOPROPERTIES()
    A_METHODS(
        A_SIGNAL(Viewport::screenshot)
    )
    A_NOENUMS()

public:
    Viewport();
    ~Viewport() override;

    void init();
    void grabScreen();
    void draw(CommandBuffer *buffer) override;
    void update(const Vector2 &position) override;
    void setSize(int width, int height) override;

    CameraController *controller() const;
    void setController(CameraController *controller);

    void setWorld(World *world);
    void setCamera(Camera *camera);

    void createMenu(Menu *menu);

    PipelineContext *pipelineContext() const;

    int gridCell() const;

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

    void screenshot(const ByteArray &pixels, int width, int height);

protected:
    Menu *createSubmenu(Menu *parent, const TString &title);
    Action *addAction(Menu *menu, const TString &name, bool checked, const std::function<void(bool)> &handler);
    void composeComponent() override;

protected:
    CameraController *m_controller;

    World *m_world;
    Camera *m_camera;

    PipelineTask *m_guiLayer;

    Outline *m_outlinePass;
    GizmoRender *m_gizmoRender;
    GridRender *m_gridRender;
    DebugRender *m_debugRender;

    PipelineContext *m_pipelineContext;

    bool m_gameView;
    bool m_gamePaused;
    bool m_liveUpdate;
    bool m_frameInProgress;
    bool m_rendering;
    bool m_initialized;
    bool m_screenInProgress;
};

#endif // VIEWPORT_H
