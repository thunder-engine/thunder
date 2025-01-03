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

#include <pipelinecontext.h>
#include <commandbuffer.h>
#include <pipelinetask.h>
#include <gizmos.h>

#include <editorsettings.h>

namespace {
    const char *gOutlineWidth("Viewport/Outline/Width");
    const char *gOutlineColor("Viewport/Outline/Color");
    const char *gGridColor("Viewport/Grid/Color");
}

class Outline : public PipelineTask {
public:
    Outline() :
            m_width(1.0f),
            m_outlineMap(Engine::objectCreate<Texture>()),
            m_outlineDepth(Engine::objectCreate<Texture>()),
            m_outlineTarget(Engine::objectCreate<RenderTarget>("Outline.Target")),
            m_resultTarget(Engine::objectCreate<RenderTarget>("Outline.ResultTarget")),
            m_controller(nullptr) {

        m_outlineDepth->setFormat(Texture::Depth);
        m_outlineDepth->setDepthBits(24);
        m_outlineDepth->setFlags(Texture::Render);

        m_outlineMap->setFormat(Texture::RGBA8);
        m_outlineMap->setFlags(Texture::Render);

        m_outlineTarget->setColorAttachment(0, m_outlineMap);
        m_outlineTarget->setDepthAttachment(m_outlineDepth);

        Material *material = Engine::loadResource<Material>(".embedded/outline.shader");
        if(material) {
            m_combineMaterial = material->createInstance();
            m_combineMaterial->setTexture("outlineMap", m_outlineMap);
        }

        setName("Outline");

        m_inputs.push_back("In");

        m_outputs.push_back(std::make_pair("Result", nullptr));

        loadSettings();
    }

    void loadSettings() {
        QColor color = EditorSettings::instance()->value(gOutlineColor, QColor(255, 128, 0, 255)).value<QColor>();
        m_color = Vector4(color.redF(), color.greenF(), color.blueF(), color.alphaF());
        m_width = EditorSettings::instance()->value(gOutlineWidth, 1.0f).toFloat();

        if(m_combineMaterial) {
            m_combineMaterial->setFloat("width", &m_width);
            m_combineMaterial->setVector4("color", &m_color);
        }
    }

    void setController(CameraController *controller) {
        m_controller = controller;
    }

    void setInput(int index, Texture *texture) override {
        if(texture) {
            m_resultTarget->setColorAttachment(index, texture);

            m_outputs.front().second = texture;
        }
    }

private:
    void exec(PipelineContext &context) override {
        if(m_combineMaterial && m_controller) {
            CommandBuffer *buffer = context.buffer();
            buffer->beginDebugMarker("Outline");

            buffer->setRenderTarget(m_outlineTarget);
            buffer->clearRenderTarget();
            RenderList filter;
            for(auto actor : m_controller->selected()) {
                for(auto it : context.culledComponents()) {
                    Renderable *component = dynamic_cast<Renderable *>(it);
                    if(component && component->actor()->isInHierarchy(static_cast<Actor *>(actor))) {
                        filter.push_back(component);
                    }
                }
            }
            context.drawRenderers(filter, CommandBuffer::RAYCAST);

            buffer->setRenderTarget(m_resultTarget);
            buffer->drawMesh(PipelineContext::defaultPlane(), 0, CommandBuffer::UI, *m_combineMaterial);

            buffer->endDebugMarker();
        }
    }

    void resize(int32_t width, int32_t height) override {
        m_outlineMap->resize(width, height);
        m_outlineDepth->resize(width, height);

        PipelineTask::resize(width, height);
    }

protected:
    Vector4 m_color;

    float m_width;

    Texture *m_outlineMap;
    Texture *m_outlineDepth;

    RenderTarget *m_outlineTarget;
    RenderTarget *m_resultTarget;

    MaterialInstance *m_combineMaterial;

    CameraController *m_controller;

};

class GridRender : public PipelineTask {
public:
    GridRender() :
            m_controller(nullptr),
            m_resultTarget(Engine::objectCreate<RenderTarget>("Grid.ResultTarget")),
            m_plane(PipelineContext::defaultPlane()),
            m_grid(nullptr),
            m_scale(1.0f) {

        Material *m = Engine::loadResource<Material>(".embedded/grid.shader");
        if(m) {
            m_grid = m->createInstance();
        }

        m_inputs.push_back("In");
        m_inputs.push_back("depthMap");

        m_outputs.push_back(std::make_pair("Result", nullptr));

        loadSettings();
    }

    void setController(CameraController *ctrl) {
        m_controller = ctrl;
    }

    void loadSettings() {
        QColor color = EditorSettings::instance()->value(gGridColor, QColor(102, 102, 102, 102)).value<QColor>();
        m_gridColor = Vector4(color.redF(), color.greenF(), color.blueF(), color.alphaF());
        if(m_grid) {
            m_grid->setVector4("mainColor", &m_gridColor);
        }
    }

    int scale() const {
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
    void exec(PipelineContext &context) override {
        Camera *camera = Camera::current();

        Transform *t = camera->transform();
        Vector3 cam = t->position();
        Vector3 pos(cam.x, 0.0f, cam.z);

        Quaternion rot;

        Vector3 planeScale(1.0f);

        enum OreintationTypes {
            XZ,
            XY,
            ZY
        };

        int orientation = XZ;

        float zoom = 0.0f;

        Vector4 gridColor(m_gridColor);

        bool ortho = camera->orthographic();
        if(ortho) {
            float height = camera->orthoSize();

            m_scale = 1;
            while(m_scale < height * 0.01f) {
                m_scale *= 10;
            }

            zoom = height / (float)m_height;
            float i;
            gridColor.w = modf(height / (float)m_scale, &i);

            planeScale = Vector3(height * camera->ratio(), height, 1.0f);

            float depth = camera->farPlane() * 0.5f;
            CameraController::ViewSide side = CameraController::ViewSide::VIEW_FRONT;
            if(m_controller) {
                side = m_controller->viewSide();
            }

            switch(side) {
                case CameraController::ViewSide::VIEW_FRONT:
                case CameraController::ViewSide::VIEW_BACK: {
                    rot = Quaternion();
                    pos = Vector3(cam.x, cam.y, cam.z + ((side == CameraController::ViewSide::VIEW_FRONT) ? -depth : depth));
                    orientation = XY;
                } break;
                case CameraController::ViewSide::VIEW_LEFT:
                case CameraController::ViewSide::VIEW_RIGHT: {
                    rot = Quaternion(Vector3(0, 1, 0), 90.0f);
                    pos = Vector3(cam.x + ((side == CameraController::ViewSide::VIEW_LEFT) ? depth : -depth), cam.y, cam.z);
                    orientation = ZY;
                } break;
                case CameraController::ViewSide::VIEW_TOP:
                case CameraController::ViewSide::VIEW_BOTTOM: {
                    rot = Quaternion(Vector3(1, 0, 0), 90.0f);
                    pos = Vector3(cam.x, cam.y + ((side == CameraController::ViewSide::VIEW_TOP) ? -depth : depth), cam.z);
                    orientation = XZ;
                } break;
                default: break;
            }
        } else {
            Ray ray = camera->castRay(0.0f, 0.0f);

            Ray::Hit hit;
            ray.intersect(Plane(Vector3(0, 0, 0), Vector3(1, 0, 0), Vector3(0, 0, 1)), &hit, true);

            float length = MIN(hit.distance, camera->farPlane()) * 0.01f;

            m_scale = 1;
            while(m_scale < length) {
                m_scale *= 10;
            }

            zoom = m_scale * 0.01f;

            planeScale = m_scale * 1000.0f;

            pos.y = 0.0f;

            rot = Quaternion(Vector3(1, 0, 0), 90.0f);
        }

        m_grid->setTransform(Matrix4(pos, rot, planeScale));
        m_grid->setBool("ortho", &ortho);
        m_grid->setInteger("scale", &m_scale);
        m_grid->setFloat("width", &zoom);
        m_grid->setInteger("orientation", &orientation);
        m_grid->setVector4("gridColor", &gridColor);

        CommandBuffer *buffer = context.buffer();

        buffer->beginDebugMarker("GridRender");

        buffer->setRenderTarget(m_resultTarget);
        buffer->drawMesh(m_plane, 0, CommandBuffer::TRANSLUCENT, *m_grid);

        buffer->endDebugMarker();
    }

private:
    Vector4 m_gridColor;

    CameraController *m_controller;

    RenderTarget *m_resultTarget;

    Mesh *m_plane;

    MaterialInstance *m_grid;

    int m_scale;

};

class GizmoRender : public PipelineTask {
public:
    GizmoRender() :
            m_controller(nullptr),
            m_spriteTarget(Engine::objectCreate<RenderTarget>("Gizmo.SpriteTarget")),
            m_geometryTarget(Engine::objectCreate<RenderTarget>("Gizmo.GeometryTarget")) {

        Gizmos::init();

        m_inputs.push_back("In");
        m_inputs.push_back("depthMap");

        m_outputs.push_back(std::make_pair("Result", nullptr));
    }

    void setController(CameraController *ctrl) {
        m_controller = ctrl;
    }

    void setInput(int index, Texture *texture) override {
        if(texture) {
            if(texture->depthBits() > 0) {
                m_spriteTarget->setDepthAttachment(texture);
            } else {
                m_spriteTarget->setColorAttachment(0, texture);
                m_geometryTarget->setColorAttachment(0, texture);

                m_outputs.front().second = texture;
            }
        }
    }

private:
    void exec(PipelineContext &context) override {
        CommandBuffer *buffer = context.buffer();
        buffer->beginDebugMarker("GizmoRender");

        Gizmos::clear();

        if(m_controller) {
            m_controller->drawHandles();
        }

        if(CommandBuffer::isInited()) {
            context.cameraReset();

            buffer->setRenderTarget(m_spriteTarget);
            Gizmos::drawSpriteBatch(buffer);

            buffer->setRenderTarget(m_geometryTarget);
            Gizmos::drawWireBatch(buffer);
            Gizmos::drawSolidBatch(buffer);
        }

        buffer->endDebugMarker();
    }

private:
    CameraController *m_controller;

    RenderTarget *m_spriteTarget;
    RenderTarget *m_geometryTarget;

};

class DebugRender : public PipelineTask {

public:
    DebugRender() :
            m_material(Engine::loadResource<Material>(".embedded/debug.shader")),
            m_mesh(PipelineContext::defaultPlane()),
            m_width(0),
            m_height(0) {

    }

    void showBuffer(const std::string &buffer) {
        m_buffers[buffer] = m_material->createInstance();
    }

    void hideBuffer(const std::string &buffer) {
        auto it = m_buffers.find(buffer);
        if(it != m_buffers.end()) {
            delete it->second;
            m_buffers.erase(it);
        }
    }

    bool isBufferVisible(const std::string &buffer) {
        auto it = m_buffers.find(buffer);
        return (it != m_buffers.end());
    }

private:
    void exec(PipelineContext &context) override {
        if(!m_buffers.empty()) {
            CommandBuffer *buffer = context.buffer();
            buffer->beginDebugMarker("DebugRender");

            int i = 0;
            for(auto &it : m_buffers) {
                it.second->setTexture("mainTexture", context.textureBuffer(it.first));

                float width = 0.5f;
                float height = 0.5f;

                Matrix4 m;
                m.scale(Vector3(width, height, 1.0));
                if(i < 4) {
                    m.mat[12] = width * 0.5f + i * width - 1.0f;
                    m.mat[13] = height * 0.5f - 1.0f;
                } else {
                    m.mat[12] = width * 0.5f + (i - 4) * width - 1.0f;
                    m.mat[13] = 1.0f - height * 0.5f;
                }
                it.second->setTransform(m);

                buffer->drawMesh(m_mesh, 0, CommandBuffer::UI, *it.second);
                i++;
            }

            buffer->endDebugMarker();
        }
    }

    void resize(int32_t width, int32_t height) override {
        m_width = width;
        m_height = height;
    }

private:
    std::map<std::string, MaterialInstance *> m_buffers;

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
        m_renderSystem(nullptr),
        m_rhiWindow(nullptr),
        m_tasksMenu(nullptr),
        m_bufferMenu(nullptr),
        m_gameView(false),
        m_gamePaused(false),
        m_liveUpdate(false),
        m_frameInProgress(false) {

    QVBoxLayout *l = new QVBoxLayout;
    l->setContentsMargins(0, 0, 0, 0);
    setLayout(l);

    setAcceptDrops(true);
    setAutoFillBackground(false);

    setMouseTracking(true);

    QObject::connect(EditorSettings::instance(), &EditorSettings::updated, this, &Viewport::onApplySettings);
}

void Viewport::init() {
    m_renderSystem = (m_gameView) ? Engine::renderSystem() : PluginManager::instance()->createRenderer();

    m_rhiWindow = m_renderSystem->createRhiWindow();

    if(m_rhiWindow) {
        m_renderSystem->init();

        m_rhiWindow->installEventFilter(this);
        layout()->addWidget(QWidget::createWindowContainer(m_rhiWindow));

        connect(m_rhiWindow, SIGNAL(draw()), this, SLOT(onDraw()), Qt::DirectConnection);

        PipelineContext *pipelineContext = m_renderSystem->pipelineContext();
        PipelineTask *lastLayer = pipelineContext->renderTasks().back();

        if(!m_gameView) {
            m_gridRender = new GridRender;
            m_gridRender->setInput(0, lastLayer->output(0));
            m_gridRender->setInput(1, pipelineContext->textureBuffer("depthMap"));
            m_gridRender->setController(m_controller);

            m_outlinePass = new Outline;
            m_outlinePass->setController(m_controller);
            m_outlinePass->setInput(0, lastLayer->output(0));

            m_gizmoRender = new GizmoRender;
            m_gizmoRender->setInput(0, lastLayer->output(0));
            m_gizmoRender->setInput(1, pipelineContext->textureBuffer("depthMap"));
            m_gizmoRender->setController(m_controller);

            if(m_controller) {
                m_controller->init(this);
            }

            pipelineContext->insertRenderTask(m_gridRender, lastLayer);
            pipelineContext->insertRenderTask(m_outlinePass, lastLayer);
            pipelineContext->insertRenderTask(m_gizmoRender, nullptr);

            Handles::init();
        }

        m_debugRender = new DebugRender;
        pipelineContext->insertRenderTask(m_debugRender, lastLayer);
    }
}

CameraController *Viewport::controller() {
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
    if(m_outlinePass) {
        m_outlinePass->loadSettings();
    }
    if(m_gridRender) {
        m_gridRender->loadSettings();
    }
}

void Viewport::onDraw() {
    if(m_frameInProgress) {
        return;
    }

    QString name = objectName();

    m_frameInProgress = true; // Recursive call protection

    if(m_world && m_liveUpdate) {
        m_world->setToBeUpdated(true);
    }

    if(m_world) {
        auto &instance = EditorPlatform::instance();

        instance.setScreenSize(size());

        bool isFocus = (QGuiApplication::focusWindow() == m_rhiWindow);

        if(m_gameView && !m_gamePaused && isFocus) {
            QPoint p = mapFromGlobal(QCursor::pos());
            instance.setMousePosition(p);
            instance.setMouseDelta(p - m_savedMousePos);

            Engine::update();

            if(instance.isMouseLocked() && Engine::isGameMode()) {
                m_savedMousePos = QPoint(width() / 2, height() / 2);
                QCursor::setPos(mapToGlobal(m_savedMousePos));

                m_rhiWindow->setCursor(Qt::BlankCursor);
            } else {
                m_savedMousePos = p;

                m_rhiWindow->setCursor(Qt::ArrowCursor);
            }
        } else {
            Engine::resourceSystem()->processEvents();

            if(m_controller) {
                Camera::setCurrent(m_controller->camera());

                m_controller->resize(width(), height());
                m_controller->move();
            } else {
                if(m_world) {
                    for(auto it : m_world->findChildren<Camera *>()) {
                        if(it->isEnabled() && it->actor()->isEnabled()) { // Get first active Camera
                            Camera::setCurrent(it);
                            break;
                        }
                    }
                }
            }

            m_renderSystem->pipelineContext()->resize(width(), height());
            m_renderSystem->update(m_world);

            if(isFocus) {
                if(m_controller) {
                    m_controller->update();
                }
                instance.update();
            }
        }
    }

    if(m_world && m_liveUpdate) {
        m_world->setToBeUpdated(true);
    }

    m_frameInProgress = false;
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

int Viewport::gridCell() {
    return m_gridRender->scale();
}

bool Viewport::isGamePaused() const {
    return m_gamePaused;
}

void Viewport::setGamePaused(bool pause) {
    m_gamePaused = pause;
    if(m_gamePaused) {
        m_rhiWindow->setCursor(Qt::ArrowCursor);
    }
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
    m_gridRender->setEnabled(enabled);
}
void Viewport::setGizmoEnabled(bool enabled) {
    m_gizmoRender->setEnabled(enabled);
}
void Viewport::setOutlineEnabled(bool enabled) {
    m_outlinePass->setEnabled(enabled);
}

void Viewport::onInProgressFlag(bool flag) {
    m_frameInProgress = flag;
}

void Viewport::addRenderTask(PipelineTask *task) {
    PipelineContext *pipelineContext = m_renderSystem->pipelineContext();
    pipelineContext->insertRenderTask(task, pipelineContext->renderTasks().front());
}

void Viewport::onBufferMenu() {
    if(m_bufferMenu && m_debugRender) {
        m_bufferMenu->clear();

        std::list<std::string> list = m_renderSystem->pipelineContext()->renderTextures();

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

        PipelineContext *context = m_renderSystem->pipelineContext();
        for(auto &it : context->renderTasks()) {
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
        std::string buffer = action->data().toString().toStdString();
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
            EditorPlatform::instance().setMousePosition(ev->pos());
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
