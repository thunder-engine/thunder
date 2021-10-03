#include "textureedit.h"
#include "ui_textureedit.h"

#include <QDir>

#include "textureconverter.h"

#include <components/scene.h>
#include <components/actor.h>
#include <components/transform.h>
#include <components/spriterender.h>
#include <components/camera.h>

#include <resources/texture.h>
#include <resources/material.h>

#include "spritecontroller.h"

#include "spriteelement.h"

TextureEdit::TextureEdit() :
        ui(new Ui::TextureEdit),
        m_Rresource(nullptr),
        m_pRender(nullptr),
        m_pConverter(new TextureConverter),
        m_Details(new SpriteElement(this)) {

    ui->setupUi(this);

    ui->preview->setScene(Engine::objectCreate<Scene>("Scene"));

    Scene *scene = ui->preview->scene();

    SpriteController *ctrl = new SpriteController(ui->preview);
    ctrl->blockRotations(true);
    ctrl->init(scene);

    connect(ctrl, &SpriteController::selectionChanged, m_Details, &SpriteElement::onSelectionChanged);

    ui->preview->setController(ctrl);

    Camera *camera = ui->preview->controller()->camera();
    if(camera) {
        camera->setOrthographic(true);
    }

    Actor *object = Engine::composeActor("SpriteRender", "Sprite", scene);
    object->transform()->setScale(Vector3(SCALE));
    m_pRender = static_cast<SpriteRender *>(object->component("SpriteRender"));
    if(m_pRender) {
        m_pRender->setMaterial(Engine::loadResource<Material>(".embedded/DefaultSprite.mtl"));
    }
}

TextureEdit::~TextureEdit() {
    delete ui;
}

bool TextureEdit::isModified() const {
    if(m_pSettings) {
        return m_pSettings->isModified();
    }
    return false;
}

void TextureEdit::loadAsset(AssetConverterSettings *settings) {
    if(m_pSettings) {
        disconnect(m_pSettings, &AssetConverterSettings::updated, this, &TextureEdit::onUpdateTemplate);
    }
    m_pSettings = static_cast<TextureImportSettings *>(settings);

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

    m_pRender->actor()->setEnabled(true);

    SpriteController *ctrl = static_cast<SpriteController *>(ui->preview->controller());
    ctrl->setImportSettings(dynamic_cast<TextureImportSettings *>(m_pSettings));
    ctrl->setSize(m_pRender->texture()->width(),
                  m_pRender->texture()->height());

    m_Details->setSettings(static_cast<TextureImportSettings*>(m_pSettings));
    m_Details->raise();

    connect(m_pSettings, &AssetConverterSettings::updated, this, &TextureEdit::onUpdateTemplate);
}

QStringList TextureEdit::suffixes() const {
    return static_cast<AssetConverter *>(m_pConverter)->suffixes();
}

void TextureEdit::onUpdateTemplate() {
    if(m_pSettings) {
        m_pConverter->convertTexture(static_cast<TextureImportSettings*>(m_pSettings), m_pRender->texture());
    }
}

void TextureEdit::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);

    QRect r = m_Details->geometry();
    r.setX(20);
    r.setY(10);
    m_Details->setGeometry(r);
}

void TextureEdit::resourceUpdated(const Resource *resource, Resource::ResourceState state) {
    if(m_Rresource == resource && state == Resource::ToBeDeleted) {
        m_pRender->actor()->setEnabled(false);
        m_Rresource = nullptr;

        SpriteController *ctrl = static_cast<SpriteController *>(ui->preview->controller());
        ctrl->setImportSettings(nullptr);
    }
}

void TextureEdit::changeEvent(QEvent *event) {
    if(event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}
