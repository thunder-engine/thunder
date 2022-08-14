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

            buffer->resetViewProjection();
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

            buffer->setScreenProjection();

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

Viewport::Viewport(QWidget *parent) :
        QWidget(parent),
        m_controller(nullptr),
        m_sceneGraph(nullptr),
        m_pipelineContext(nullptr),
        m_outlinePass(nullptr),
        m_rhiWindow(nullptr),
        m_postMenu(nullptr),
        m_lightMenu(nullptr),
        m_bufferMenu(nullptr) {

    QVBoxLayout *l = new QVBoxLayout;
    l->setContentsMargins(0, 0, 0, 0);
    setLayout(l);

    RenderSystem *render = PluginManager::instance()->render();
    if(render) {
        m_rhiWindow = PluginManager::instance()->render()->createRhiWindow();
        m_rhiWindow->installEventFilter(this);
        layout()->addWidget(QWidget::createWindowContainer(m_rhiWindow));
    }

    setAcceptDrops(true);
    setAutoFillBackground(false);

    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    QObject::connect(SettingsManager::instance(), &SettingsManager::updated, this, &Viewport::onApplySettings);
}

void Viewport::init() {
    if(m_rhiWindow) {
        connect(m_rhiWindow, SIGNAL(draw()), this, SLOT(onDraw()), Qt::DirectConnection);
    }
}

void Viewport::setController(CameraCtrl *ctrl) {
    m_controller = ctrl;
}

void Viewport::setSceneGraph(SceneGraph *sceneGraph) {
    m_sceneGraph = sceneGraph;

    //RenderSystem *render = PluginManager::instance()->render();
    //m_pipelineContext = render->pipelineContext(m_sceneGraph);

    Camera *camera = m_controller->camera();
    if(camera) {
        m_pipelineContext = camera->pipeline();
        m_outlinePass = new Outline;
        m_outlinePass->setController(m_controller);
        m_outlinePass->loadSettings();
        m_pipelineContext->addRenderPass(m_outlinePass);
        for(auto it : m_pipelineContext->renderPasses()) {
            SettingsManager::instance()->registerProperty(qPrintable(QString(postSettings) + it->name()), it->isEnabled());
        }

        Handles::init();
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
    if(m_controller) {
        Camera *camera = m_controller->camera();
        if(camera) {
            PipelineContext *pipe = camera->pipeline();
            for(auto it : pipe->renderPasses()) {
                it->setEnabled(SettingsManager::instance()->property(qPrintable(QString(postSettings) + it->name())).toBool());
            }
        }
    }
    if(m_outlinePass) {
        m_outlinePass->loadSettings();
    }
}

void Viewport::onDraw() {
    if(m_controller) {
        m_controller->update();
        m_controller->resize(width(), height());

        Camera *camera = m_controller->camera();
        if(camera) {
            PipelineContext *pipe = camera->pipeline();
            pipe->resize(width(), height());
        }
        Camera::setCurrent(camera);
    }
    if(m_sceneGraph) {
        Engine::resourceSystem()->processEvents();

        RenderSystem *render = PluginManager::instance()->render();
        if(render) {
            render->update(m_sceneGraph);
        }
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

        PipelineContext *pipe = nullptr;
        Camera *camera = m_controller->camera();
        if(camera) {
            pipe = camera->pipeline();
        } else {
            return;
        }

        list<string> list = pipe->renderTextures();
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

        PipelineContext *pipe = nullptr;
        Camera *camera = m_controller->camera();
        if(camera) {
            pipe = camera->pipeline();
        } else {
            return;
        }

        for(auto &it : pipe->renderPasses()) {
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
        PipelineContext *pipe = nullptr;
        Camera *camera = m_controller->camera();
        if(camera) {
            pipe = camera->pipeline();
        } else {
            return;
        }

        pipe->setDebugTexture(action->data().toString().toStdString());
    }
}

void Viewport::onPostEffectChanged(bool checked) {
    QAction *action = qobject_cast<QAction *>(QObject::sender());
    if(action) {
        PipelineContext *pipe = nullptr;
        Camera *camera = m_controller->camera();
        if(camera) {
            pipe = camera->pipeline();
        } else {
            return;
        }

        for(auto &it : pipe->renderPasses()) {
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
