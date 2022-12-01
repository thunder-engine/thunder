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

#include <pipelinecontext.h>

#include <systems/rendersystem.h>

#include <editor/pluginmanager.h>

#include "../converter/textureconverter.h"

#include "spritecontroller.h"
#include "spriteelement.h"

TextureEdit::TextureEdit() :
        ui(new Ui::TextureEdit),
        m_resource(nullptr),
        m_render(nullptr),
        m_converter(new TextureConverter),
        m_graph(Engine::objectCreate<SceneGraph>("SceneGraph")) {

    ui->setupUi(this);

    m_controller = new SpriteController(this);
    m_controller->frontSide();
    m_controller->blockRotations(true);

    ui->viewport->setController(m_controller);
    ui->viewport->init();

    connect(m_controller, &SpriteController::selectionChanged, ui->widget, &SpriteElement::onSelectionChanged);
    connect(m_controller, &SpriteController::setCursor, ui->viewport, &Viewport::onCursorSet, Qt::DirectConnection);
    connect(m_controller, &SpriteController::unsetCursor, ui->viewport, &Viewport::onCursorUnset, Qt::DirectConnection);

    Camera *camera = m_controller->camera();
    if(camera) {
        camera->setOrthographic(true);
    }

    Actor *object = Engine::composeActor("SpriteRender", "Sprite", m_graph);
    m_render = static_cast<SpriteRender *>(object->component("SpriteRender"));

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
    ui->viewport->setSceneGraph(m_graph);

    if(!m_settings.isEmpty()) {
        disconnect(m_settings.first(), &AssetConverterSettings::updated, this, &TextureEdit::onUpdateTemplate);
    }
    m_settings = { settings };

    if(m_resource) {
        m_resource->unsubscribe(this);
    }

    m_resource = Engine::loadResource<Resource>(qPrintable(settings->destination()));
    if(m_resource) {
        m_resource->subscribe(this);
    }

    Sprite *sprite = dynamic_cast<Sprite *>(m_resource);
    if(sprite) {
         m_render->setSprite(sprite);
    } else {
        Texture *texture = dynamic_cast<Texture *>(m_resource);
        if(texture) {
            m_render->setTexture(texture);
        }
    }

    float ratio = (float)m_render->texture()->width() / (float)m_render->texture()->height();
    Transform *t = m_render->actor()->transform();
    t->setScale(Vector3(SCALE * ratio, SCALE, SCALE));

    m_render->actor()->setEnabled(true);

    m_controller->setImportSettings(dynamic_cast<TextureImportSettings *>(m_settings.first()));
    m_controller->setSize(m_render->texture()->width(), m_render->texture()->height());

    ui->widget->setSettings(static_cast<TextureImportSettings*>(m_settings.first()));

    connect(m_settings.first(), &AssetConverterSettings::updated, this, &TextureEdit::onUpdateTemplate);
}

QStringList TextureEdit::suffixes() const {
    return static_cast<AssetConverter *>(m_converter)->suffixes();
}

void TextureEdit::onUpdateTemplate() {
    if(!m_settings.isEmpty()) {
        m_converter->convertTexture(static_cast<TextureImportSettings*>(m_settings.first()), m_render->texture());
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
    if(m_resource == resource && state == Resource::ToBeDeleted) {
        m_render->actor()->setEnabled(false);
        m_resource = nullptr;

        m_controller->setImportSettings(nullptr);
    }
}

void TextureEdit::changeEvent(QEvent *event) {
    if(event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}
