#include "editor/viewport/viewport.h"
#include "editor/viewport/cameracontroller.h"
#include "editor/viewport/handles.h"

#include "editor/pluginmanager.h"

#include "editor/editorplatform.h"

#include <QWindow>
#include <QMenu>
#include <QVBoxLayout>
#include <QGuiApplication>
#include <QRegularExpression>

#include <systems/rendersystem.h>
#include <systems/resourcesystem.h>

#include <components/actor.h>
#include <components/camera.h>
#include <components/transform.h>
#include <components/world.h>

#include <resources/rendertarget.h>

#include <pipelinetasks/guilayer.h>

#include <pipelinecontext.h>
#include <commandbuffer.h>
#include <pipelinetask.h>
#include <gizmos.h>

#include <settingsmanager.h>

namespace {
    const char *postSettings("Graphics/");
    const char *outlineWidth("General/Colors/Outline_Width");
    const char *outlineColor("General/Colors/Outline_Color");
    const char *gridColor("General/Colors/Grid_Color");
}

class Outline : public PipelineTask {
public:
    Outline() :
            m_width(1.0f),
            m_outlineMap(Engine::objectCreate<Texture>()),
            m_outlineDepth(Engine::objectCreate<Texture>()),
            m_outlineTarget(Engine::objectCreate<RenderTarget>()),
            m_resultTarget(Engine::objectCreate<RenderTarget>()),
            m_controller(nullptr) {

        m_outlineDepth->setFormat(Texture::Depth);
        m_outlineDepth->setDepthBits(24);

        m_outlineMap->setFormat(Texture::RGBA8);

        m_outlineTarget->setColorAttachment(0, m_outlineMap);
        m_outlineTarget->setDepthAttachment(m_outlineDepth);

        Material *material = Engine::loadResource<Material>(".embedded/outline.shader");
        if(material) {
            m_material = material->createInstance();
            m_material->setTexture("outlineMap", m_outlineMap);
        }

        setName("Outline");

        SettingsManager::instance()->registerProperty(outlineWidth, 1.0f);
        SettingsManager::instance()->registerProperty(outlineColor, QColor(255, 128, 0, 255));

        m_inputs.push_back("In");
        m_inputs.push_back("depthMap");

        m_outputs.push_back(make_pair("Result", nullptr));
    }

    void loadSettings() {
        QColor color = SettingsManager::instance()->property(qPrintable(outlineColor)).value<QColor>();
        m_color = Vector4(color.redF(), color.greenF(), color.blueF(), color.alphaF());
        m_width = SettingsManager::instance()->property(qPrintable(outlineWidth)).toFloat();

        if(m_material) {
            m_material->setFloat("uni.width", &m_width);
            m_material->setVector4("uni.color", &m_color);
        }
    }

    void setController(CameraController *controller) {
        m_controller = controller;
    }

    void setInput(int index, Texture *texture) override {
        if(texture) {
            if(texture->depthBits() > 0) {
                m_resultTarget->setDepthAttachment(texture);
            } else {
                m_resultTarget->setColorAttachment(index, texture);

                m_outputs.front().second = texture;
            }
        }
    }

private:
    void exec(PipelineContext *context) override {
        if(m_material && m_controller) {
            CommandBuffer *buffer = context->buffer();

            buffer->setRenderTarget(m_outlineTarget);
            buffer->clearRenderTarget();
            RenderList filter;
            for(auto actor : m_controller->selected()) {
                for(auto it : context->culledComponents()) {
                    Renderable *component = dynamic_cast<Renderable *>(it);
                    if(component && component->actor()->isInHierarchy(static_cast<Actor *>(actor))) {
                        filter.push_back(component);
                    }
                }
            }
            context->drawRenderers(CommandBuffer::RAYCAST, filter);

            buffer->setRenderTarget(m_resultTarget);
            buffer->drawMesh(Matrix4(), PipelineContext::defaultPlane(), 0, CommandBuffer::UI, m_material);
        }
    }

    void resize(int32_t width, int32_t height) override {
        m_outlineMap->setWidth(width);
        m_outlineMap->setHeight(height);

        m_outlineDepth->setWidth(width);
        m_outlineDepth->setHeight(height);

        PipelineTask::resize(width, height);
    }

protected:
    Vector4 m_color;

    float m_width;

    Texture *m_outlineMap;
    Texture *m_outlineDepth;

    RenderTarget *m_outlineTarget;
    RenderTarget *m_resultTarget;

    MaterialInstance *m_material;

    CameraController *m_controller;

};

class GridRender : public PipelineTask {
public:
    GridRender() :
            m_controller(nullptr),
            m_resultTarget(Engine::objectCreate<RenderTarget>()),
            m_plane(PipelineContext::defaultPlane()),
            m_grid(nullptr),
            m_scale(1.0f) {

        Material *m = Engine::loadResource<Material>(".embedded/grid.shader");
        if(m) {
            m_grid = m->createInstance();
        }

        SettingsManager::instance()->registerProperty(gridColor, QColor(102, 102, 102, 102));

        m_inputs.push_back("In");
        m_inputs.push_back("depthMap");

        m_outputs.push_back(make_pair("Result", nullptr));
    }

    void setController(CameraController *ctrl) {
        m_controller = ctrl;
    }

    void loadSettings() {
        QColor color = SettingsManager::instance()->property(gridColor).value<QColor>();
        m_gridColor = Vector4(color.redF(), color.greenF(), color.blueF(), color.alphaF());
    }

    float scale() const {
        return m_scale;
    }

    void setInput(int index, Texture *texture) override {
        if(texture) {
            if(texture->depthBits() > 0) {
                m_resultTarget->setDepthAttachment(texture);
            } else {
                m_resultTarget->setColorAttachment(0, texture);

                m_outputs.front().second = texture;
            }
        }
    }

private:
    void exec(PipelineContext *context) override {
        Camera *camera = Camera::current();

        Transform *t = camera->transform();
        Vector3 cam = t->position();
        Vector3 pos(cam.x, 0.0f, cam.z);

        Quaternion rot;

        m_scale = 1.0f;
        float width = 0.5f;

        bool ortho = camera->orthographic();
        if(ortho) {
            float length = camera->orthoSize() * 10.0f;

            m_scale = 0.01f;
            while(m_scale < length) {
                m_scale *= 10.0f;
            }

            width = length / m_scale;
            float scale = m_scale * 0.1f;

            float depth = camera->farPlane() - camera->nearPlane();
            CameraController::ViewSide side = m_controller->viewSide();
            switch(side) {
                case CameraController::ViewSide::VIEW_FRONT:
                case CameraController::ViewSide::VIEW_BACK: {
                    rot = Quaternion();
                    pos = Vector3(cam.x, cam.y, cam.z + ((side == CameraController::ViewSide::VIEW_FRONT) ? -depth : depth));
                    pos = Vector3(scale * int32_t(pos.x / scale),
                                  scale * int32_t(pos.y / scale),
                                  pos.z);
                } break;
                case CameraController::ViewSide::VIEW_LEFT:
                case CameraController::ViewSide::VIEW_RIGHT: {
                    rot = Quaternion(Vector3(0, 1, 0), 90.0f);
                    pos = Vector3(cam.x + ((side == CameraController::ViewSide::VIEW_LEFT) ? depth : -depth), cam.y, cam.z);
                    pos = Vector3(pos.x,
                                  scale * int32_t(pos.y / scale),
                                  scale * int32_t(pos.z / scale));
                } break;
                case CameraController::ViewSide::VIEW_TOP:
                case CameraController::ViewSide::VIEW_BOTTOM: {
                    rot = Quaternion(Vector3(1, 0, 0), 90.0f);
                    pos = Vector3(cam.x, cam.y + ((side == CameraController::ViewSide::VIEW_TOP) ? -depth : depth), cam.z);
                    pos = Vector3(scale * int32_t(pos.x / scale),
                                  pos.y,
                                  scale * int32_t(pos.z / scale));
                } break;
                default: break;
            }
        } else {
            Ray ray = camera->castRay(0.0f, 0.0f);

            Ray::Hit hit;
            ray.intersect(Plane(Vector3(0, 0, 0), Vector3(1, 0, 0), Vector3(0, 0, 1)), &hit, true);

            float length = MIN(hit.distance, camera->farPlane()) * 10.0f;

            m_scale = 10.0f;
            while(m_scale < length) {
                m_scale *= 10.0f;
            }

            width = length / m_scale;
            float scale = m_scale * 0.1f;
            pos = Vector3(scale * int32_t(pos.x / scale),
                          0.0f,
                          scale * int32_t(pos.z / scale));

            rot = Quaternion(Vector3(1, 0, 0), 90.0f);
        }

        m_grid->setBool("uni.ortho", &ortho);
        m_grid->setFloat("uni.scale", &m_scale);
        m_grid->setFloat("uni.width", &width);

        CommandBuffer *buffer = context->buffer();

        buffer->setRenderTarget(m_resultTarget);
        buffer->setColor(m_gridColor);
        buffer->drawMesh(Matrix4(pos, rot, m_scale), m_plane, 0, CommandBuffer::TRANSLUCENT, m_grid);
        buffer->setColor(Vector4(1.0f));
    }

private:
    Vector4 m_gridColor;

    CameraController *m_controller;

    RenderTarget *m_resultTarget;

    Mesh *m_plane;

    MaterialInstance *m_grid;

    float m_scale;

};

class GizmoRender : public PipelineTask {
public:
    GizmoRender() :
            m_controller(nullptr) {

        Gizmos::init();
    }

    void setController(CameraController *ctrl) {
        m_controller = ctrl;
    }

private:
    void exec(PipelineContext *context) override {
        CommandBuffer *buffer = context->buffer();

        Gizmos::beginDraw();

        Camera *cam = Camera::current();
        if(cam) {
            Gizmos::setViewProjection(cam->viewMatrix(), cam->projectionMatrix());
        }

        if(m_controller) {
            m_controller->drawHandles();
        }

        Gizmos::endDraw(buffer);
    }

private:
    CameraController *m_controller;

};

class DebugRender : public PipelineTask {

public:
    DebugRender() :
            m_material(Engine::loadResource<Material>(".embedded/debug.shader")),
            m_mesh(PipelineContext::defaultPlane()),
            m_width(0),
            m_height(0) {

    }

    void showBuffer(const string &buffer) {
        m_buffers[buffer] = m_material->createInstance();
    }

    void hideBuffer(const string &buffer) {
        auto it = m_buffers.find(buffer);
        if(it != m_buffers.end()) {
            delete it->second;
            m_buffers.erase(it);
        }
    }

    bool isBufferVisible(const string &buffer) {
        auto it = m_buffers.find(buffer);
        return (it != m_buffers.end());
    }

private:
    void exec(PipelineContext *context) override {
        if(!m_buffers.empty()) {
            CommandBuffer *buffer = context->buffer();
            buffer->setScreenProjection(0, 0, m_width, m_height);

            int i = 0;
            for(auto &it : m_buffers) {
                it.second->setTexture("texture0", context->textureBuffer(it.first));

                float width = m_width * 0.25f;
                float height = m_height * 0.25f;

                Matrix4 m;
                m.scale(Vector3(width, height, 1.0));
                if(i < 4) {
                    m.mat[12] = width * 0.5f + i * width;
                    m.mat[13] = height * 0.5f;
                } else {
                    m.mat[12] = width * 0.5f + (i - 4) * width;
                    m.mat[13] = m_height - height * 0.5f;
                }
                buffer->drawMesh(m, m_mesh, 0, CommandBuffer::UI, it.second);
                i++;
            }

            buffer->resetViewProjection();
        }
    }

    void resize(int32_t width, int32_t height) override {
        m_width = width;
        m_height = height;
    }

private:
    map<string, MaterialInstance *> m_buffers;

    Material *m_material;
    Mesh *m_mesh;

    int32_t m_width;
    int32_t m_height;

};

Viewport::Viewport(QWidget *parent) :
        QWidget(parent),
        m_controller(nullptr),
        m_world(nullptr),
        m_outlinePass(nullptr),
        m_gizmoRender(nullptr),
        m_gridRender(nullptr),
        m_debugRender(nullptr),
        m_renderSystem(PluginManager::instance()->createRenderer()),
        m_rhiWindow(m_renderSystem->createRhiWindow()),
        m_tasksMenu(nullptr),
        m_bufferMenu(nullptr) {

    QVBoxLayout *l = new QVBoxLayout;
    l->setContentsMargins(0, 0, 0, 0);
    setLayout(l);

    m_rhiWindow->installEventFilter(this);
    layout()->addWidget(QWidget::createWindowContainer(m_rhiWindow));

    setAcceptDrops(true);
    setAutoFillBackground(false);

    setMouseTracking(true);

    QObject::connect(SettingsManager::instance(), &SettingsManager::updated, this, &Viewport::onApplySettings);
}

void Viewport::init() {
    if(m_rhiWindow) {
        m_renderSystem->init();
        connect(m_rhiWindow, SIGNAL(draw()), this, SLOT(onDraw()), Qt::DirectConnection);

        setWorldSpaceGui(true);

        PipelineContext *pipelineContext = m_renderSystem->pipelineContext();

        PipelineTask *lastLayer = pipelineContext->renderTasks().back();

        m_gridRender = new GridRender;
        m_gridRender->setInput(0, lastLayer->output(0));
        m_gridRender->setInput(1, pipelineContext->textureBuffer("depthMap"));
        m_gridRender->setController(m_controller);
        m_gridRender->loadSettings();

        m_outlinePass = new Outline;
        m_outlinePass->setController(m_controller);
        m_outlinePass->setInput(0, lastLayer->output(0));
        m_outlinePass->setInput(1, pipelineContext->textureBuffer("depthMap"));
        m_outlinePass->loadSettings();

        m_gizmoRender = new GizmoRender;
        m_gizmoRender->setInput(0, lastLayer->output(0));
        m_gizmoRender->setInput(1, pipelineContext->textureBuffer("depthMap"));
        m_gizmoRender->setController(m_controller);

        m_debugRender = new DebugRender;

        if(m_controller) {
            m_controller->init(this);
        }

        pipelineContext->insertRenderTask(m_gridRender, lastLayer);
        pipelineContext->insertRenderTask(m_outlinePass, lastLayer);
        pipelineContext->insertRenderTask(m_gizmoRender, lastLayer);
        pipelineContext->insertRenderTask(m_debugRender, lastLayer);

        for(auto it : pipelineContext->renderTasks()) {
            if(!it->name().empty()) {
                SettingsManager::instance()->registerProperty(qPrintable(QString(postSettings) + it->name().c_str()), it->isEnabled());
            }
        }

        Handles::init();
    }
}

CameraController *Viewport::controllder() {
    return m_controller;
}

void Viewport::setController(CameraController *ctrl) {
    m_controller = ctrl;

    connect(m_controller, &CameraController::setCursor, this, &Viewport::onCursorSet);
    connect(m_controller, &CameraController::unsetCursor, this, &Viewport::onCursorUnset);
}

void Viewport::setWorld(World *world) {
    if(m_world != world) {
        m_world = world;

        if(m_controller) {
            m_controller->setActiveRootObject(m_world);
        }
    }
}

void Viewport::onCursorSet(const QCursor &cursor) {
    if(m_rhiWindow) {
        m_rhiWindow->setCursor(cursor);
    }
}

void Viewport::onCursorUnset() {
    if(m_rhiWindow) {
        m_rhiWindow->unsetCursor();
    }
}

void Viewport::onApplySettings() {
    PipelineContext *pipelineContext = m_renderSystem->pipelineContext();
    if(pipelineContext) {
        for(auto it : pipelineContext->renderTasks()) {
            if(!it->name().empty()) {
                it->setEnabled(SettingsManager::instance()->property(qPrintable(QString(postSettings) + it->name().c_str())).toBool());
            }
        }
    }
    if(m_outlinePass) {
        m_outlinePass->loadSettings();
    }
    if(m_gridRender) {
        m_gridRender->loadSettings();
    }
}

void Viewport::onDraw() {
    if(m_controller) {
        Camera::setCurrent(m_controller->camera());

        m_controller->resize(width(), height());
        m_controller->move();

        m_renderSystem->pipelineContext()->resize(width(), height());
    }

    if(m_world) {
        Engine::resourceSystem()->processEvents();

        m_renderSystem->update(m_world);

        if(m_controller && QGuiApplication::focusWindow() == m_rhiWindow) {
            m_controller->update();
            EditorPlatform::instance().update();
        }

        Camera::setCurrent(nullptr);
    }
}

void Viewport::createMenu(QMenu *menu) {
    m_tasksMenu = menu->addMenu(tr("Render Tasks"));
    m_bufferMenu = menu->addMenu(tr("Buffer Visualization"));

    fillTasksMenu(m_tasksMenu);

    QObject::connect(m_bufferMenu, &QMenu::aboutToShow, this, &Viewport::onBufferMenu);
}

PipelineContext *Viewport::pipelineContext() const {
    return m_renderSystem->pipelineContext();
}

float Viewport::gridCell() {
    return m_gridRender->scale() * 0.001f;
}

void Viewport::setGridEnabled(bool enabled) {
    m_gridRender->setEnabled(enabled);
}
void Viewport::setGizmoEnabled(bool enabled) {
    m_gizmoRender->setEnabled(enabled);
}
void Viewport::setOutlineEnabled(bool enabled) {
    m_outlinePass->setEnabled(enabled);
}

void Viewport::addRenderTask(PipelineTask *task) {
    PipelineContext *pipelineContext = m_renderSystem->pipelineContext();
    pipelineContext->insertRenderTask(task, pipelineContext->renderTasks().front());
}

void Viewport::setWorldSpaceGui(bool flag) {
    PipelineContext *pipelineContext = m_renderSystem->pipelineContext();

    for(auto it : pipelineContext->renderTasks()) {
        GuiLayer *gui = dynamic_cast<GuiLayer *>(it);
        if(gui) {
            gui->showUiAsSceneView(flag);
            break;
        }
    }
}

void Viewport::onBufferMenu() {
    if(m_bufferMenu) {
        m_bufferMenu->clear();

        list<string> list = m_renderSystem->pipelineContext()->renderTextures();

        for(auto &it : list) {
            static QRegularExpression regExp1 {"(.)([A-Z][a-z]+)"};
            static QRegularExpression regExp2 {"([a-z0-9])([A-Z])"};

            QString result(it.c_str());
            result.replace(regExp1, "\\1 \\2");
            result.replace(regExp2, "\\1 \\2");
            result.replace(0, 1, result[0].toUpper());

            QAction *action = m_bufferMenu->addAction(result);
            action->setData(it.c_str());
            action->setCheckable(true);
            action->setChecked(m_debugRender->isBufferVisible(it.c_str()));
            QObject::connect(action, &QAction::triggered, this, &Viewport::onBufferChanged);
        }
    }
}

void Viewport::fillTasksMenu(QMenu *menu) {
    if(menu) {
        menu->clear();

        for(auto &it : m_renderSystem->pipelineContext()->renderTasks()) {
            static QRegularExpression regExp1 {"(.)([A-Z][a-z]+)"};
            static QRegularExpression regExp2 {"([a-z0-9])([A-Z])"};

            QString result(it->name().c_str());
            if(result.isEmpty()) {
                continue;
            }
            result.replace(regExp1, "\\1 \\2");
            result.replace(regExp2, "\\1 \\2");
            result.replace(0, 1, result[0].toUpper());

            QAction *action = menu->addAction(result);
            action->setCheckable(true);
            action->setChecked(it->isEnabled());
            action->setData(it->name().c_str());

            QObject::connect(action, &QAction::toggled, this, &Viewport::onPostEffectChanged);
        }
    }
}

void Viewport::onBufferChanged(bool checked) {
    QAction *action = qobject_cast<QAction *>(QObject::sender());
    if(action) {
        string buffer = action->data().toString().toStdString();
        if(checked) {
            m_debugRender->showBuffer(buffer);
        } else {
            m_debugRender->hideBuffer(buffer);
        }
    }
}

void Viewport::onPostEffectChanged(bool checked) {
    QAction *action = qobject_cast<QAction *>(QObject::sender());
    if(action) {
        for(auto &it : m_renderSystem->pipelineContext()->renderTasks()) {
            if(action->data().toString().toStdString() == it->name()) {
                it->setEnabled(checked);
                SettingsManager::instance()->setProperty(qPrintable(QString(postSettings) + it->name().c_str()), checked);
            }
        }
    }
}

bool Viewport::eventFilter(QObject *object, QEvent *event) {
    bool isFocus = (QGuiApplication::focusWindow() == m_rhiWindow);

    switch(event->type()) {
    case QEvent::DragEnter: emit dragEnter(static_cast<QDragEnterEvent *>(event)); return true;
    case QEvent::DragLeave: emit dragLeave(static_cast<QDragLeaveEvent *>(event)); return true;
    case QEvent::DragMove: emit dragMove(static_cast<QDragMoveEvent *>(event)); return true;
    case QEvent::Drop: emit drop(static_cast<QDropEvent *>(event)); setFocus(); return true;
    case QEvent::KeyPress: {
        if(isFocus) {
            QKeyEvent *ev = static_cast<QKeyEvent *>(event);
            EditorPlatform::instance().setKeys(ev->key(), ev->text(), false, ev->isAutoRepeat());
        }
        return true;
    }
    case QEvent::KeyRelease: {
        if(isFocus) {
            QKeyEvent *ev = static_cast<QKeyEvent *>(event);
            EditorPlatform::instance().setKeys(ev->key(), "", true, ev->isAutoRepeat());
        }
        return true;
    }
    case QEvent::Wheel: {
        if(isFocus) {
            QWheelEvent *ev = static_cast<QWheelEvent *>(event);
            EditorPlatform::instance().setMouseScrollDelta(ev->delta());
        }
        return true;
    }
    case QEvent::MouseButtonPress: {
        if(isFocus) {
            QMouseEvent *ev = static_cast<QMouseEvent *>(event);
            EditorPlatform::instance().setScreenSize(size());
            EditorPlatform::instance().setMousePosition(ev->pos());
            EditorPlatform::instance().setMouseButtons(ev->button(), PRESS);
        }
        return true;
    }
    case QEvent::MouseButtonRelease: {
        if(isFocus) {
            QMouseEvent *ev = static_cast<QMouseEvent *>(event);
            EditorPlatform::instance().setMouseButtons(ev->button(), RELEASE);
        }
        return true;
    }
    case QEvent::MouseMove: {
        if(isFocus) {
            QMouseEvent *ev = static_cast<QMouseEvent *>(event);
            EditorPlatform::instance().setScreenSize(size());
            EditorPlatform::instance().setMousePosition(ev->pos());
        }
        return true;
    }
    default: break;
    }

    return QObject::eventFilter(object, event);
}

void Viewport::resizeEvent(QResizeEvent *event) {
    EditorPlatform::instance().setScreenSize(event->size());
}
