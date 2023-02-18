#include "editor/viewport/viewport.h"
#include "editor/viewport/cameractrl.h"
#include "editor/viewport/handles.h"

#include "editor/pluginmanager.h"

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
#include <pipelinepass.h>
#include <gizmos.h>

#include <settingsmanager.h>

namespace {
    const char *postSettings("Graphics/");
    const char *outlineWidth("General/Colors/Outline_Width");
    const char *outlineColor("General/Colors/Outline_Color");
    const char *gridColor("General/Colors/Grid_Color");
}

class Outline : public PipelinePass {
public:
    enum Inputs {
        Source,
        Depth
    };

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

    void setInput(uint32_t index, Texture *texture) override {
        switch(index) {
        case Source: {
            if(m_material) {
                m_material->setTexture("rgbMap", texture);
            }

        } break;
        case Depth: {
            m_resultTarget->setDepthAttachment(texture);
        } break;
        default: break;
        }
    }

private:
    Texture *draw(Texture *source, PipelineContext *context) override {
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

            m_resultTarget->setColorAttachment(Source, source);
            buffer->setRenderTarget(m_resultTarget);
            buffer->drawMesh(Matrix4(), PipelineContext::defaultPlane(), 0, CommandBuffer::UI, m_material);
        }
        return source;
    }

    void resize(int32_t width, int32_t height) override {
        m_outlineMap->setWidth(width);
        m_outlineMap->setHeight(height);

        m_outlineDepth->setWidth(width);
        m_outlineDepth->setHeight(height);

        PipelinePass::resize(width, height);
    }

protected:
    Vector4 m_color;

    float m_width;

    Texture *m_outlineMap;
    Texture *m_outlineDepth;

    RenderTarget *m_outlineTarget;
    RenderTarget *m_resultTarget;

    MaterialInstance *m_material;

    CameraCtrl *m_controller;

};

class GridRender : public PipelinePass {
public:
    enum Inputs {
        Source,
        Depth
    };

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
    }

    void setController(CameraCtrl *ctrl) {
        m_controller = ctrl;
    }

    void loadSettings() {
        QColor color = SettingsManager::instance()->property(gridColor).value<QColor>();
        m_gridColor = Vector4(color.redF(), color.greenF(), color.blueF(), color.alphaF());
    }

    float scale() const {
        return m_scale;
    }

    void setInput(uint32_t index, Texture *texture) override {
        switch(index) {
        case Depth: {
            m_resultTarget->setDepthAttachment(texture);
        } break;
        default: break;
        }
    }

private:
    uint32_t layer() const override {
        return CommandBuffer::TRANSLUCENT;
    }

    Texture *draw(Texture *source, PipelineContext *context) override {
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
            float y = abs(cam.y) * 10.0f;
            m_scale = 10.0f;
            while(m_scale < y) {
                m_scale *= 10.0f;
            }

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

        m_resultTarget->setColorAttachment(Source, source);

        buffer->setRenderTarget(m_resultTarget);
        buffer->setColor(m_gridColor);
        buffer->drawMesh(Matrix4(pos, rot, m_scale), m_plane, 0, CommandBuffer::TRANSLUCENT, m_grid);
        buffer->setColor(Vector4(1.0f));

        return source;
    }

private:
    Vector4 m_gridColor;

    CameraCtrl *m_controller;

    RenderTarget *m_resultTarget;

    Mesh *m_plane;

    MaterialInstance *m_grid;

    float m_scale;

};

class GizmoRender : public PipelinePass {
public:
    enum Inputs {
        Source,
        Depth
    };

public:
    GizmoRender() :
            m_controller(nullptr) {

        Gizmos::init();
    }

    void setController(CameraCtrl *ctrl) {
        m_controller = ctrl;
    }

private:
    uint32_t layer() const override {
        return CommandBuffer::UI;
    }

    Texture *draw(Texture *source, PipelineContext *context) override {
        CommandBuffer *buffer = context->buffer();

        Gizmos::beginDraw();
        if(m_controller) {
            m_controller->drawHandles();
        }
        Gizmos::endDraw(buffer);

        return source;
    }

private:
    CameraCtrl *m_controller;

};

class DebugRender : public PipelinePass {

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
    uint32_t layer() const override {
        return CommandBuffer::UI;
    }

    Texture *draw(Texture *source, PipelineContext *context) override {
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

        return source;
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
        m_sceneGraph(nullptr),
        m_outlinePass(nullptr),
        m_gizmoRender(nullptr),
        m_gridRender(nullptr),
        m_debugRender(nullptr),
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
    //setFocusPolicy(Qt::StrongFocus);

    QObject::connect(SettingsManager::instance(), &SettingsManager::updated, this, &Viewport::onApplySettings);
}

void Viewport::init() {
    if(m_rhiWindow) {
        m_renderSystem->init();
        connect(m_rhiWindow, SIGNAL(draw()), this, SLOT(onDraw()), Qt::DirectConnection);

        PipelineContext *pipelineContext = m_renderSystem->pipelineContext();

        pipelineContext->showUiAsSceneView();

        m_gridRender = new GridRender;
        m_gridRender->setInput(GridRender::Depth, pipelineContext->textureBuffer("depthMap"));
        m_gridRender->setController(m_controller);
        m_gridRender->loadSettings();

        m_outlinePass = new Outline;
        m_outlinePass->setController(m_controller);
        m_outlinePass->setInput(Outline::Source, pipelineContext->textureBuffer("emissiveMap"));
        m_outlinePass->setInput(Outline::Depth, pipelineContext->textureBuffer("depthMap"));
        m_outlinePass->loadSettings();

        m_gizmoRender = new GizmoRender;
        m_gizmoRender->setInput(GizmoRender::Source, pipelineContext->textureBuffer("emissiveMap"));
        m_gizmoRender->setInput(GizmoRender::Depth, pipelineContext->textureBuffer("depthMap"));
        m_gizmoRender->setController(m_controller);

        m_debugRender = new DebugRender;

        if(m_controller) {
            m_controller->init(this);
        }

        PipelinePass *guiLayer = pipelineContext->renderPasses().back();

        pipelineContext->insertRenderPass(m_gridRender, guiLayer);
        pipelineContext->insertRenderPass(m_outlinePass, guiLayer);
        pipelineContext->insertRenderPass(m_gizmoRender, guiLayer);
        pipelineContext->insertRenderPass(m_debugRender, guiLayer);

        for(auto it : pipelineContext->renderPasses()) {
            if(!it->name().empty()) {
                SettingsManager::instance()->registerProperty(qPrintable(QString(postSettings) + it->name().c_str()), it->isEnabled());
            }
        }

        Handles::init();
    }
}

void Viewport::setController(CameraCtrl *ctrl) {
    m_controller = ctrl;
}

void Viewport::setWorld(World *sceneGraph) {
    if(m_sceneGraph != sceneGraph) {
        m_sceneGraph = sceneGraph;

        m_controller->setActiveRootObject(m_sceneGraph);
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
        for(auto it : pipelineContext->renderPasses()) {
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
        m_controller->update();
        m_controller->resize(width(), height());

        m_renderSystem->pipelineContext()->resize(width(), height());

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

void Viewport::addPass(PipelinePass *pass) {
    PipelineContext *pipelineContext = m_renderSystem->pipelineContext();
    pipelineContext->insertRenderPass(pass, pipelineContext->renderPasses().front());
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

void Viewport::fillEffectMenu(QMenu *menu, uint32_t layers) {
    if(menu) {
        menu->clear();

        for(auto &it : m_renderSystem->pipelineContext()->renderPasses()) {
            if(it->layer() & layers) {
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
        for(auto &it : m_renderSystem->pipelineContext()->renderPasses()) {
            if(action->data().toString().toStdString() == it->name()) {
                it->setEnabled(checked);
                SettingsManager::instance()->setProperty(qPrintable(QString(postSettings) + it->name().c_str()), checked);
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
