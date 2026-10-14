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
#include "viewport/viewport.h"
#include "viewport/cameracontroller.h"
#include "viewport/handles.h"

#include "viewport/tasks/debugrender.h"
#include "viewport/tasks/gizmorender.h"
#include "viewport/tasks/gridrender.h"
#include "viewport/tasks/outlinerender.h"

#include <action.h>
#include <actor.h>
#include <camera.h>
#include <menu.h>
#include <world.h>

#include <resources/pipeline.h>
#include <resources/texture.h>
#include <resources/rendertarget.h>

#include <pipelinecontext.h>
#include <input.h>
#include <log.h>

#include <cctype>
#include <string>

TString menuItemText(const TString &name) {
    std::string text(name.data());
    std::string result;
    for(size_t i = 0; i < text.size(); ++i) {
        const unsigned char current = static_cast<unsigned char>(text[i]);
        if(i > 0 && std::isupper(current)) {
            const unsigned char previous = static_cast<unsigned char>(text[i - 1]);
            const bool wordBoundary = std::islower(previous) || std::isdigit(previous) ||
                    (std::isupper(previous) && i + 1 < text.size() &&
                     std::islower(static_cast<unsigned char>(text[i + 1])));
            if(wordBoundary) {
                result.push_back(' ');
            }
        }
        result.push_back(static_cast<char>(current));
    }
    if(!result.empty()) {
        result[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(result[0])));
    }
    return TString(result);
}

Viewport::Viewport() :
        m_controller(nullptr),
        m_world(nullptr),
        m_camera(nullptr),
        m_guiLayer(nullptr),
        m_outlinePass(nullptr),
        m_gizmoRender(nullptr),
        m_gridRender(nullptr),
        m_debugRender(nullptr),
        m_pipelineContext(nullptr),
        m_gameView(false),
        m_gamePaused(false),
        m_liveUpdate(false),
        m_frameInProgress(false),
        m_rendering(false),
        m_initialized(false),
        m_screenInProgress(false) {

    Texture *output = Canvas::texture();
    RenderTarget *target = renderTarget();
    if(output) {
        output->setFlags(output->flags() | Texture::Feedback);
    }
    if(target) {
        target->setClearColor(Vector4(0.2f, 0.2f, 0.2f, 1.0f));
        target->setFlags(RenderTarget::ClearColor);
    }
}

Viewport::~Viewport() {
}

void Viewport::init() {
    if(m_initialized) {
        return;
    }

    if(!Canvas::texture() || !renderTarget()) {
        aError() << "Unable to create the Viewport render target.";
        return;
    }

    Pipeline *pipeline = Engine::loadResource<Pipeline>(
        Engine::value(".pipeline", ".embedded/Deferred.pipeline").toString());
    if(!pipeline) {
        aError() << "Unable to load the Viewport render pipeline.";
        return;
    }

    m_pipelineContext = Engine::objectCreate<PipelineContext>("PipelineContext");
    if(!m_pipelineContext) {
        aError() << "Unable to create the Viewport render context.";
        return;
    }
    m_pipelineContext->setPipeline(pipeline);
    m_pipelineContext->setDefaultTarget(renderTarget());
    m_pipelineContext->setWorld(m_world);
    if(m_pipelineContext->renderTasks().empty()) {
        aError() << "The Viewport render pipeline contains no render tasks.";
        m_pipelineContext->deleteLater();
        m_pipelineContext = nullptr;
        return;
    }

    m_guiLayer = m_pipelineContext->renderTasks().back();
    m_debugRender = new DebugRender;

    if(!m_gameView) {
        m_gridRender = new GridRender;
        m_gridRender->setController(m_controller);
        m_gridRender->setInput(0, m_guiLayer->output(0));
        m_gridRender->setInput(1, m_pipelineContext->textureBuffer("depthMap"));

        m_outlinePass = new Outline;
        m_outlinePass->setController(m_controller);
        m_outlinePass->setInput(0, m_guiLayer->output(0));

        m_gizmoRender = new GizmoRender;
        m_gizmoRender->setController(m_controller);
        m_gizmoRender->setInput(0, m_guiLayer->output(0));
        m_gizmoRender->setInput(1, m_pipelineContext->textureBuffer("depthMap"));

        m_pipelineContext->insertRenderTask(m_gridRender, m_guiLayer);
        m_pipelineContext->insertRenderTask(m_outlinePass, m_guiLayer);
        m_pipelineContext->insertRenderTask(m_gizmoRender);

        Handles::init();
    }

    m_pipelineContext->insertRenderTask(m_debugRender, m_guiLayer);

    m_initialized = true;
}

void Viewport::composeComponent() {
    Widget::composeComponent();
    init();
}

void Viewport::draw(CommandBuffer *buffer) {
    if(m_frameInProgress) {
        return;
    }
    m_frameInProgress = true;
    m_frameInProgress = true;

    if(m_world && m_pipelineContext && !m_rendering) {
        Widget *rect = this;
        Vector2 size = rect->size();
        if(size.x > 0.0f && size.y > 0.0f) {
            Camera *previousCamera = Camera::current();
            Camera *camera = nullptr;
            if(m_gameView) {
                camera = Camera::findActiveCamera(m_world);
            } else {
                if(m_controller) {
                    Camera::setCurrent(m_controller->camera());

                    m_controller->resize(static_cast<int32_t>(size.x), static_cast<int32_t>(size.y));
                    m_controller->move();
                    camera = m_controller->camera();
                } else {
                    camera = m_camera;
                }
            }

            if(camera) {
                m_rendering = true;
                m_world->setActive(m_liveUpdate && (!m_gameView || !m_gamePaused));
                m_pipelineContext->setWorld(m_world);
                Camera::setCurrent(camera);
                const Matrix4 &m = rect->worldTransform();
                m_pipelineContext->setPositionOffset(Vector2(m[12], m[13]));
                m_pipelineContext->resize(static_cast<int32_t>(size.x), static_cast<int32_t>(size.y));
                m_pipelineContext->draw(camera, buffer);
                Camera::setCurrent(previousCamera);
                buffer->setCameraProperties(previousCamera);

                m_rendering = false;
            }
        }
    }

    Canvas::draw(buffer);
    if(m_screenInProgress) {
        Texture *output = texture();
        if(output && output->width() > 0 && output->height() > 0) {
            output->readPixels(0, 0, output->width(), output->height());
            screenshot(output->getPixels(0), output->width(), output->height());
        }
        m_screenInProgress = false;
    }
    m_frameInProgress = false;
}

void Viewport::grabScreen() {
    m_screenInProgress = true;
}

void Viewport::screenshot(const ByteArray &pixels, int width, int height) {
    //emitSignal(_SIGNAL(screenshot(ByteArray,int,int)), pixels, width, height);
}

void Viewport::update(const Vector2 &position) {
    Canvas::update(position);

    Widget *root = this;
    if(m_controller && !m_gameView && root) {
        Vector2 size = root->size();
        if(size.x > 0.0f && size.y > 0.0f) {
            m_controller->resize(static_cast<int32_t>(size.x), static_cast<int32_t>(size.y));
            if(root->isHovered(position)) {
                m_controller->update();
            }
        }
    }

    if(m_world) {
        m_world->setActive(m_liveUpdate && (!m_gameView || !m_gamePaused));
    }

    // TODO: Port EditorPlatform game-view updates and mouse-lock handling from LegacyViewport.
}

void Viewport::setSize(int width, int height) {
    Canvas::setSize(width, height);
}

void Viewport::setWorld(World *world) {
    if(m_world != world) {
        m_world = world;
        if(m_controller) {
            m_controller->setActiveRootObject(world);
        }
        if(m_pipelineContext) {
            m_pipelineContext->setWorld(world);
        }
    }
}

CameraController *Viewport::controller() const {
    return m_controller;
}

void Viewport::setController(CameraController *controller) {
    if(m_controller == controller) {
        return;
    }

    m_controller = controller;
    if(m_controller) {
        m_controller->setActiveRootObject(m_world);
    }
    if(m_gridRender) {
        m_gridRender->setController(m_controller);
    }
    if(m_outlinePass) {
        m_outlinePass->setController(m_controller);
    }
    if(m_gizmoRender) {
        m_gizmoRender->setController(m_controller);
    }
}

void Viewport::setCamera(Camera *camera) {
    m_camera = camera;
}

void Viewport::createMenu(Menu *menu) {
    if(!menu || !m_pipelineContext) {
        return;
    }

    Menu *tasksMenu = createSubmenu(menu, "Render Tasks");
    for(auto task : m_pipelineContext->renderTasks()) {
        if(!tasksMenu) {
            break;
        }
        if(!task->name().isEmpty()) {
            addAction(tasksMenu, task->name(), task->isEnabled(), [task](bool checked) {
                task->setEnabled(checked);
            });
        }
    }

    if(m_debugRender) {
        Menu *bufferMenu = createSubmenu(menu, "Buffer Visualization");
        for(auto &name : m_pipelineContext->renderTextures()) {
            if(!bufferMenu) {
                break;
            }
            if(!name.isEmpty()) {
                addAction(bufferMenu, name, m_debugRender->isBufferVisible(name.data()), [this, name](bool checked) {
                    if(m_debugRender) {
                        m_debugRender->trackBuffer(name.data(), checked);
                    }
                });
            }
        }
    }
}

PipelineContext *Viewport::pipelineContext() const {
    return m_pipelineContext;
}

int Viewport::gridCell() const {
    return m_gridRender ? m_gridRender->scale() : 0;
}

bool Viewport::isGamePaused() const {
    return m_gamePaused;
}

void Viewport::setGamePaused(bool pause) {
    m_gamePaused = pause;
}

bool Viewport::isLiveUpdate() const {
    return m_liveUpdate;
}

void Viewport::setLiveUpdate(bool update) {
    m_liveUpdate = update;
}

void Viewport::setGameView(bool enabled) {
    m_gameView = enabled;
}

void Viewport::setGridEnabled(bool enabled) {
    if(m_gridRender) {
        m_gridRender->setEnabled(enabled);
    }
}

void Viewport::setGizmoEnabled(bool enabled) {
    if(m_gizmoRender) {
        m_gizmoRender->setEnabled(enabled);
    }
}

void Viewport::setOutlineEnabled(bool enabled) {
    if(m_outlinePass) {
        m_outlinePass->setEnabled(enabled);
    }
}

void Viewport::setGuiEnabled(bool enabled) {
    if(m_guiLayer) {
        m_guiLayer->setEnabled(enabled);
    }
}

void Viewport::showCube(bool enabled) {
    if(m_gizmoRender) {
        m_gizmoRender->showCube(enabled);
    }
}

void Viewport::showGizmos(bool enabled) {
    if(m_gizmoRender) {
        m_gizmoRender->showGizmos(enabled);
    }
}

void Viewport::addRenderTask(PipelineTask *task) {
    if(m_pipelineContext && task) {
        const auto &tasks = m_pipelineContext->renderTasks();
        m_pipelineContext->insertRenderTask(task, tasks.empty() ? nullptr : tasks.front());
    }
}

Menu *Viewport::createSubmenu(Menu *parent, const TString &title) {
    if(!parent) {
        return nullptr;
    }
    Menu *submenu = Widget::createWidget<Menu>(title, parent);
    if(!submenu) {
        aError() << "Unable to create the Viewport menu submenu.";
        return nullptr;
    }
    parent->addSubmenu(title, submenu);
    return submenu;
}

Action *Viewport::addAction(Menu *menu, const TString &name, bool checked, const std::function<void(bool)> &handler) {
    if(!menu) {
        return nullptr;
    }
    Action *action = Widget::createWidget<Action>(name, menu);
    if(!action) {
        aError() << "Unable to create the Viewport menu action.";
        return nullptr;
    }

    action->setText(menuItemText(name));
    action->setCheckable(true);
    action->setChecked(checked);
    action->setTriggeredHandler([action, handler]() {
        handler(action->isChecked());
    });
    return menu->addAction(action);
}
