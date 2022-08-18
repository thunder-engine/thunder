#include "editor/viewport/viewport.h"
#include "editor/viewport/cameractrl.h"
#include "editor/viewport/handles.h"

#include "editor/pluginmanager.h"

#include <QWindow>
#include <QMenu>
#include <QVBoxLayout>
#include <QGuiApplication>
#include <QRegularExpression>

#include <renderpass.h>

#include <systems/rendersystem.h>
#include <systems/resourcesystem.h>

#include <components/actor.h>
#include <components/camera.h>
#include <components/transform.h>

#include <resources/rendertarget.h>

#include <pipelinecontext.h>
#include <commandbuffer.h>

#include <settingsmanager.h>

namespace {
    const char *postSettings("Graphics/");
    const char *outlineWidth("General/Colors/Outline_Width");
    const char *outlineColor("General/Colors/Outline_Color");
    const char *gridColor("General/Colors/Grid_Color");
}

class Outline : public RenderPass {
public:
    Outline() :
        m_width(1.0f) {

        m_resultTexture = Engine::objectCreate<Texture>();
        m_resultTexture->setFormat(Texture::RGBA8);

        m_resultTarget->setColorAttachment(0, m_resultTexture);

        m_outlineDepth = Engine::objectCreate<Texture>();
        m_outlineDepth->setFormat(Texture::Depth);
        m_outlineDepth->setDepthBits(24);

        m_outlineMap = Engine::objectCreate<Texture>();
        m_outlineMap->setFormat(Texture::RGBA8);

        m_outlineTarget = Engine::objectCreate<RenderTarget>();
        m_outlineTarget->setColorAttachment(0, m_outlineMap);
        m_outlineTarget->setDepthAttachment(m_outlineDepth);

        Material *material = Engine::loadResource<Material>(".embedded/outline.shader");
        if(material) {
            m_material = material->createInstance();
            m_material->setTexture("outlineMap", m_outlineMap);
        }

        SettingsManager::instance()->registerProperty(outlineWidth, 1.0f);
        SettingsManager::instance()->registerProperty(outlineColor, QColor(255, 128, 0, 255));
    }

    Texture *draw(Texture *source, PipelineContext *context) override {
        if(m_enabled && m_material) {
            m_material->setTexture("rgbMap", source);

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
            buffer->drawMesh(Matrix4(), m_mesh, 0, CommandBuffer::UI, m_material);

            return m_resultTexture;
        }

        return source;
    }

    void resize(int32_t width, int32_t height) override {
        m_outlineMap->setWidth(width);
        m_outlineMap->setHeight(height);

        m_outlineDepth->setWidth(width);
        m_outlineDepth->setHeight(height);

        RenderPass::resize(width, height);
    }

    const char *name() const override {
        return "Outline";
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

    void setController(CameraCtrl *controller) {
        m_controller = controller;
    }

protected:
    Vector4 m_color;

    float m_width;

    Texture *m_outlineMap;
    Texture *m_outlineDepth;

    RenderTarget *m_outlineTarget;

    CameraCtrl *m_controller;

};

class GizmoRender : public RenderPass {
public:
    GizmoRender() :
            m_controller(nullptr),
            m_plane(Engine::loadResource<Mesh>(".embedded/plane.fbx/Plane001")),
            m_grid(nullptr) {

        Material *m = Engine::loadResource<Material>(".embedded/grid.shader");
        if(m) {
            m_grid = m->createInstance();
        }

        SettingsManager::instance()->registerProperty(gridColor, QColor(102, 102, 102, 102));
    }

    void setController(CameraCtrl *ctrl) {
        m_controller = ctrl;
    }

    void loadSettings() {
        QColor color = SettingsManager::instance()->property(gridColor).value<QColor>();
        m_gridColor = Vector4(color.redF(), color.greenF(), color.blueF(), color.alphaF());
    }

private:
    Texture *draw(Texture *source, PipelineContext *context) override {
        if(context->debugTexture() == nullptr) {
            // Draw handles
            CommandBuffer *buffer = context->buffer();
            drawGrid(*Camera::current(), buffer);

            Handles::beginDraw(buffer);
            if(m_controller) {
                m_controller->drawHandles();
            }
            Handles::endDraw();
        }
        return source;
    }

    void drawGrid(Camera &camera, CommandBuffer *buffer) {
        Transform *t = camera.actor()->transform();
        Vector3 cam = t->position();
        Vector3 pos(cam.x, 0.0f, cam.z);

        Quaternion rot;

        float scale = 1.0f;
        float width = 0.5f;

        bool ortho = camera.orthographic();
        if(ortho) {
            float length = camera.orthoSize() * 10.0f;

            scale = 0.01f;
            while(scale < length) {
                scale *= 10.0f;
            }

            width = length / scale;

            float depth = camera.farPlane() - camera.nearPlane();
            CameraCtrl::ViewSide side = m_controller->viewSide();
            switch(side) {
                case CameraCtrl::ViewSide::VIEW_FRONT:
                case CameraCtrl::ViewSide::VIEW_BACK: {
                    rot = Quaternion();
                    pos = Vector3(cam.x, cam.y, cam.z + ((side == CameraCtrl::ViewSide::VIEW_FRONT) ? -depth : depth));
                    pos = Vector3(scale * int32_t(pos.x / scale),
                                  scale * int32_t(pos.y / scale),
                                  pos.z);
                } break;
                case CameraCtrl::ViewSide::VIEW_LEFT:
                case CameraCtrl::ViewSide::VIEW_RIGHT: {
                    rot = Quaternion(Vector3(0, 1, 0), 90.0f);
                    pos = Vector3(cam.x + ((side == CameraCtrl::ViewSide::VIEW_LEFT) ? depth : -depth), cam.y, cam.z);
                    pos = Vector3(pos.x,
                                  scale * int32_t(pos.y / scale),
                                  scale * int32_t(pos.z / scale));
                } break;
                case CameraCtrl::ViewSide::VIEW_TOP:
                case CameraCtrl::ViewSide::VIEW_BOTTOM: {
                    rot = Quaternion(Vector3(1, 0, 0), 90.0f);
                    pos = Vector3(cam.x, cam.y + ((side == CameraCtrl::ViewSide::VIEW_TOP) ? -depth : depth), cam.z);
                    pos = Vector3(scale * int32_t(pos.x / scale),
                                  pos.y,
                                  scale * int32_t(pos.z / scale));
                } break;
                default: break;
            }
        } else {
            scale = 100.0f;

            pos = Vector3(scale * int32_t(pos.x / scale),
                          0.0f,
                          scale * int32_t(pos.z / scale));

            rot = Quaternion(Vector3(1, 0, 0), 90.0f);
        }

        m_grid->setBool("uni.ortho", &ortho);
        m_grid->setFloat("uni.scale", &scale);
        m_grid->setFloat("uni.width", &width);

        buffer->setColor(m_gridColor);
        buffer->drawMesh(Matrix4(pos, rot, scale), m_plane, 0, CommandBuffer::TRANSLUCENT, m_grid);
        buffer->setColor(Vector4(1.0f));
    }

private:
    Vector4 m_gridColor;

    CameraCtrl *m_controller;

    Mesh *m_plane;

    MaterialInstance *m_grid;

};

Viewport::Viewport(QWidget *parent) :
        QWidget(parent),
        m_controller(nullptr),
        m_sceneGraph(nullptr),
        m_pipelineContext(nullptr),
        m_outlinePass(nullptr),
        m_gizmoRender(nullptr),
        m_renderSystem(PluginManager::instance()->createRenderer()),
        m_rhiWindow(m_renderSystem->createRhiWindow()),
        m_postMenu(nullptr),
        m_lightMenu(nullptr),
        m_bufferMenu(nullptr) {

    QVBoxLayout *l = new QVBoxLayout;
    l->setContentsMargins(0, 0, 0, 0);
    setLayout(l);

    m_rhiWindow->installEventFilter(this);
    layout()->addWidget(QWidget::createWindowContainer(m_rhiWindow));

    setAcceptDrops(true);
    setAutoFillBackground(false);

    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    QObject::connect(SettingsManager::instance(), &SettingsManager::updated, this, &Viewport::onApplySettings);
}

void Viewport::init() {
    if(m_rhiWindow) {
        m_renderSystem->init();
        connect(m_rhiWindow, SIGNAL(draw()), this, SLOT(onDraw()), Qt::DirectConnection);
    }
}

void Viewport::setController(CameraCtrl *ctrl) {
    m_controller = ctrl;
}

void Viewport::setSceneGraph(SceneGraph *sceneGraph) {
    if(m_sceneGraph != sceneGraph) {
        m_sceneGraph = sceneGraph;

        //RenderSystem *render = PluginManager::instance()->render();
        //m_pipelineContext = render->pipelineContext(m_sceneGraph);

        Camera *camera = m_controller->camera();
        if(camera) {
            m_outlinePass = new Outline;
            m_outlinePass->setController(m_controller);
            m_outlinePass->loadSettings();

            m_gizmoRender = new GizmoRender;
            m_gizmoRender->setController(m_controller);
            m_gizmoRender->loadSettings();

            m_pipelineContext = camera->pipeline();
            m_pipelineContext->addRenderPass(m_outlinePass);
            m_pipelineContext->addRenderPass(m_gizmoRender);
            m_pipelineContext->showUiAsSceneView();
            for(auto it : m_pipelineContext->renderPasses()) {
                SettingsManager::instance()->registerProperty(qPrintable(QString(postSettings) + it->name()), it->isEnabled());
            }

            Handles::init();
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
    if(m_pipelineContext) {
        for(auto it : m_pipelineContext->renderPasses()) {
            it->setEnabled(SettingsManager::instance()->property(qPrintable(QString(postSettings) + it->name())).toBool());
        }
    }
    if(m_outlinePass) {
        m_outlinePass->loadSettings();
    }
    if(m_gizmoRender) {
        m_gizmoRender->loadSettings();
    }
}

void Viewport::onDraw() {
    if(m_controller) {
        m_controller->update();
        m_controller->resize(width(), height());

        m_pipelineContext->resize(width(), height());

        Camera::setCurrent(m_controller->camera());
    }
    if(m_sceneGraph) {
        Engine::resourceSystem()->processEvents();

        m_renderSystem->update(m_sceneGraph);

        Camera::setCurrent(nullptr);
    }
}

void Viewport::createMenu(QMenu *menu) {
    m_postMenu = menu->addMenu(tr("Post Processing"));
    m_lightMenu = menu->addMenu(tr("Lighting Features"));
    m_bufferMenu = menu->addMenu(tr("Buffer Visualization"));

    fillEffectMenu(m_lightMenu, CommandBuffer::DEFAULT | CommandBuffer::LIGHT);
    fillEffectMenu(m_postMenu, CommandBuffer::TRANSLUCENT | CommandBuffer::UI);

    QObject::connect(m_bufferMenu, &QMenu::aboutToShow, this, &Viewport::onBufferMenu);
}

void Viewport::onBufferMenu() {
    if(m_bufferMenu) {
        m_bufferMenu->clear();

        list<string> list = m_pipelineContext->renderTextures();
        list.push_front("Final Buffer");

        bool first = true;
        for(auto &it : list) {
            static QRegularExpression regExp1 {"(.)([A-Z][a-z]+)"};
            static QRegularExpression regExp2 {"([a-z0-9])([A-Z])"};

            QString result(it.c_str());
            result.replace(regExp1, "\\1 \\2");
            result.replace(regExp2, "\\1 \\2");
            result.replace(0, 1, result[0].toUpper());

            QAction *action = m_bufferMenu->addAction(result);
            action->setData(it.c_str());
            QObject::connect(action, &QAction::triggered, this, &Viewport::onBufferChanged);
            if(first) {
                m_bufferMenu->addSeparator();
                first = false;
            }
        }
    }
}

void Viewport::fillEffectMenu(QMenu *menu, uint32_t layers) {
    if(menu) {
        menu->clear();

        for(auto &it : m_pipelineContext->renderPasses()) {
            if(it->layer() & layers) {
                static QRegularExpression regExp1 {"(.)([A-Z][a-z]+)"};
                static QRegularExpression regExp2 {"([a-z0-9])([A-Z])"};

                QString result = it->name();
                if(result.isEmpty()) {
                    continue;
                }
                result.replace(regExp1, "\\1 \\2");
                result.replace(regExp2, "\\1 \\2");
                result.replace(0, 1, result[0].toUpper());

                QAction *action = menu->addAction(result);
                action->setCheckable(true);
                action->setChecked(it->isEnabled());
                action->setData(it->name());

                QObject::connect(action, &QAction::toggled, this, &Viewport::onPostEffectChanged);
            }
        }
    }
}

void Viewport::onBufferChanged() {
    QAction *action = qobject_cast<QAction *>(QObject::sender());
    if(action) {
        m_pipelineContext->setDebugTexture(action->data().toString().toStdString());
    }
}

void Viewport::onPostEffectChanged(bool checked) {
    QAction *action = qobject_cast<QAction *>(QObject::sender());
    if(action) {
        for(auto &it : m_pipelineContext->renderPasses()) {
            if(action->data().toString() == it->name()) {
                it->setEnabled(checked);
                SettingsManager::instance()->setProperty(qPrintable(QString(postSettings) + it->name()), checked);
            }
        }
    }
}

bool Viewport::eventFilter(QObject *object, QEvent *event) {
    switch(event->type()) {
    case QEvent::DragEnter: emit dragEnter(static_cast<QDragEnterEvent *>(event)); return true;
    case QEvent::DragLeave: emit dragLeave(static_cast<QDragLeaveEvent *>(event)); return true;
    case QEvent::DragMove: emit dragMove(static_cast<QDragMoveEvent *>(event)); return true;
    case QEvent::Drop: emit drop(static_cast<QDropEvent *>(event)); setFocus(); return true;
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
    case QEvent::Wheel:
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseMove: {
        // Workaround for the modal dialogs on top of RHI window and events propagation on to RHI
        if(m_controller && m_rhiWindow == QGuiApplication::focusWindow()) {
            m_controller->onInputEvent(static_cast<QMouseEvent *>(event));
        }
        return true;
    }
    default: break;
    }

    return QObject::eventFilter(object, event);
}
