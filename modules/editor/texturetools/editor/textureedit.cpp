#include "textureedit.h"
#include "ui_textureedit.h"

#include <QWindow>
#include <QHBoxLayout>

#include <components/scenegraph.h>
#include <components/actor.h>
#include <components/transform.h>
#include <components/spriterender.h>
#include <components/camera.h>

#include <resources/texture.h>
#include <resources/material.h>
#include <resources/pipeline.h>

#include <systems/rendersystem.h>

#include <editor/pluginmanager.h>

#include "../converter/textureconverter.h"

#include "spritecontroller.h"
#include "spriteelement.h"

TextureEdit::TextureEdit() :
        ui(new Ui::TextureEdit),
        m_Rresource(nullptr),
        m_pRender(nullptr),
        m_pConverter(new TextureConverter),
        m_pScene(Engine::objectCreate<SceneGraph>("SceneGraph")) {

    ui->setupUi(this);

    m_pController = new SpriteController(this);
    m_pController->blockRotations(true);
    m_pController->init();

    connect(m_pController, &SpriteController::selectionChanged, ui->widget, &SpriteElement::onSelectionChanged);
    connect(m_pController, &SpriteController::setCursor, this, &TextureEdit::onCursorSet, Qt::DirectConnection);
    connect(m_pController, &SpriteController::unsetCursor, this, &TextureEdit::onCursorUnset, Qt::DirectConnection);

    Camera *camera = m_pController->camera();
    if(camera) {
        camera->setOrthographic(true);
    }

    Actor *object = Engine::composeActor("SpriteRender", "Sprite", m_pScene);
    m_pRender = static_cast<SpriteRender *>(object->component("SpriteRender"));
    if(m_pRender) {
        m_pRender->setMaterial(Engine::loadResource<Material>(".embedded/DefaultSprite.mtl"));
    }

    m_pRHIWindow = PluginManager::instance()->render()->createRhiWindow();
    m_pRHIWindow->installEventFilter(this);
    static_cast<QHBoxLayout *>(layout())->insertWidget(0, QWidget::createWindowContainer(m_pRHIWindow));

    connect(m_pRHIWindow, SIGNAL(draw()), this, SLOT(onDraw()), Qt::DirectConnection);

    setAcceptDrops(true);
    setMouseTracking(true);
}

TextureEdit::~TextureEdit() {
    delete ui;
}

bool TextureEdit::isModified() const {
    if(!m_settings.isEmpty()) {
        return m_settings.first()->isModified();
    }
    return false;
}

void TextureEdit::loadAsset(AssetConverterSettings *settings) {
    if(!m_settings.isEmpty()) {
        disconnect(m_settings.first(), &AssetConverterSettings::updated, this, &TextureEdit::onUpdateTemplate);
    }
    m_settings = { settings };

    if(m_Rresource) {
        m_Rresource->unsubscribe(this);
    }

    m_Rresource = Engine::loadResource<Resource>(qPrintable(settings->destination()));
    if(m_Rresource) {
        m_Rresource->subscribe(this);
    }

    Sprite *sprite = dynamic_cast<Sprite *>(m_Rresource);
    if(sprite) {
         m_pRender->setSprite(sprite);
    } else {
        Texture *texture = dynamic_cast<Texture *>(m_Rresource);
        if(texture) {
            m_pRender->setTexture(texture);
        }
    }

    float ratio = (float)m_pRender->texture()->width() / (float)m_pRender->texture()->height();
    Transform *t = m_pRender->actor()->transform();
    t->setScale(Vector3(SCALE * ratio, SCALE, SCALE));

    m_pRender->actor()->setEnabled(true);

    m_pController->setImportSettings(dynamic_cast<TextureImportSettings *>(m_settings.first()));
    m_pController->setSize(m_pRender->texture()->width(), m_pRender->texture()->height());

    ui->widget->setSettings(static_cast<TextureImportSettings*>(m_settings.first()));

    connect(m_settings.first(), &AssetConverterSettings::updated, this, &TextureEdit::onUpdateTemplate);
}

QStringList TextureEdit::suffixes() const {
    return static_cast<AssetConverter *>(m_pConverter)->suffixes();
}

void TextureEdit::onUpdateTemplate() {
    if(!m_settings.isEmpty()) {
        m_pConverter->convertTexture(static_cast<TextureImportSettings*>(m_settings.first()), m_pRender->texture());
    }
}

void TextureEdit::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);

    QRect r = ui->widget->geometry();
    r.setX(20);
    r.setY(10);
    ui->widget->setGeometry(r);
}

void TextureEdit::resourceUpdated(const Resource *resource, Resource::ResourceState state) {
    if(m_Rresource == resource && state == Resource::ToBeDeleted) {
        m_pRender->actor()->setEnabled(false);
        m_Rresource = nullptr;

        m_pController->setImportSettings(nullptr);
    }
}

void TextureEdit::onCursorSet(const QCursor &cursor) {
    m_pRHIWindow->setCursor(cursor);
}

void TextureEdit::onCursorUnset() {
    m_pRHIWindow->unsetCursor();
}

void TextureEdit::onDraw() {
    if(m_pController) {
        m_pController->update();

        Camera *camera = m_pController->camera();
        if(camera) {
            Pipeline *pipe = camera->pipeline();
            pipe->resize(width(), height());
        }
        Camera::setCurrent(camera);
    }
    if(m_pScene) {
        Engine::resourceSystem()->processEvents();

        RenderSystem *render = PluginManager::instance()->render();
        if(render) {
            render->update(m_pScene);
        }
        Camera::setCurrent(nullptr);
    }
}

void TextureEdit::changeEvent(QEvent *event) {
    if(event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}

bool TextureEdit::eventFilter(QObject *object, QEvent *event) {
     // Workaround for the modal dialogs on top of RHI window and events propagation on to RHI
    if(m_pRHIWindow == QGuiApplication::focusWindow()) {
        switch(event->type()) {
        case QEvent::Wheel:
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::MouseMove: {
            if(m_pController) {
                m_pController->onInputEvent(static_cast<QInputEvent *>(event));
            }
            return true;
        }
        default: break;
        }
    }
    return QObject::eventFilter(object, event);
}
