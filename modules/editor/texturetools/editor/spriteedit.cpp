#include "spriteedit.h"
#include "ui_spriteedit.h"

#include <components/world.h>
#include <components/actor.h>
#include <components/transform.h>
#include <components/spriterender.h>
#include <components/camera.h>
#include <components/scene.h>

#include <resources/texture.h>
#include <resources/material.h>

#include "../converter/textureconverter.h"

#include "spritecontroller.h"
#include "spriteelement.h"

namespace {
    const char *gSpriteRender("SpriteRender");
};

SpriteEdit::SpriteEdit() :
        ui(new Ui::SpriteEdit),
        m_resource(nullptr),
        m_render(nullptr),
        m_converter(new TextureConverter),
        m_world(Engine::objectCreate<World>("World")),
        m_scene(Engine::objectCreate<Scene>("Scene", m_world)),
        m_controller(new SpriteController(this)) {

    ui->setupUi(this);

    m_controller->doRotation(Vector3());
    m_controller->setGridAxis(CameraController::Axis::Z);
    m_controller->blockRotations(true);

    ui->viewport->setController(m_controller);
    ui->viewport->setWorld(m_world);
    ui->viewport->init(); // must be called after all options set
    ui->viewport->setGridEnabled(false);

    connect(m_controller, &SpriteController::itemsSelected, this, &SpriteEdit::itemsSelected);
    connect(m_controller, &SpriteController::updated, this, &SpriteEdit::updated);

    Camera *camera = m_controller->camera();
    if(camera) {
        camera->setOrthographic(true);
    }

    Actor *object = Engine::composeActor(gSpriteRender, gSpriteRender, m_scene);
    m_render = object->getComponent<SpriteRender>();
    m_render->setLayer(2);

    object = Engine::composeActor(gSpriteRender, "CheckerBoard", m_scene);
    m_checker = object->getComponent<SpriteRender>();
    m_checker->setMaterial(Engine::loadResource<Material>(".embedded/checkerboard.shader"));

    setAcceptDrops(true);
    setMouseTracking(true);
}

SpriteEdit::~SpriteEdit() {
    if(m_resource) {
        m_resource->unsubscribe(this);
    }

    delete m_world;

    delete ui;
}

bool SpriteEdit::isModified() const {
    if(!m_settings.isEmpty()) {
        return m_settings.first()->isModified();
    }
    return false;
}

void SpriteEdit::loadAsset(AssetConverterSettings *settings) {
    if(m_settings.contains(settings)) {
        return;
    }

    if(!m_settings.isEmpty()) {
        disconnect(m_settings.first(), &AssetConverterSettings::updated, this, &SpriteEdit::onUpdateTemplate);
    }
    AssetEditor::loadAsset(settings);

    if(m_resource) {
        m_resource->unsubscribe(this);
    }

    m_resource = Engine::loadResource<Resource>(qPrintable(settings->destination()));
    if(m_resource) {
        m_resource->subscribe(&SpriteEdit::resourceUpdated, this);
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

    Texture *texture = m_render->texture();
    Vector3 size(texture->width(), texture->height(), 0);

    Transform *renderTransform = m_render->transform();
    renderTransform->setScale(size);
    renderTransform->setPosition(size * 0.5f);

    Vector2 scale(texture->width() / 20.0f, texture->height() / 20.0f);
    m_checker->materialInstance()->setVector2("scale", &scale);

    Transform *checkerTransform = m_checker->transform();
    checkerTransform->setScale(size);
    checkerTransform->setPosition(size * 0.5f);

    m_render->actor()->setEnabled(true);

    m_controller->setSettings(dynamic_cast<TextureImportSettings *>(m_settings.first()));
    m_controller->setSize(texture->width(), texture->height());

    connect(m_settings.first(), &AssetConverterSettings::updated, this, &SpriteEdit::onUpdateTemplate);
}

void SpriteEdit::saveAsset(const QString &) {
    m_settings.first()->saveSettings();
}

QStringList SpriteEdit::suffixes() const {
    return static_cast<AssetConverter *>(m_converter)->suffixes();
}

void SpriteEdit::onUpdateTemplate() {
    if(!m_settings.isEmpty()) {
        m_converter->convertTexture(m_render->texture(), static_cast<TextureImportSettings*>(m_settings.first()));
    }
}

void SpriteEdit::resourceUpdated(int state, void *ptr) {
    if(state == Resource::ToBeDeleted) {
        SpriteEdit *p = static_cast<SpriteEdit *>(ptr);
        p->m_render->actor()->setEnabled(false);
        p->m_resource = nullptr;

        p->m_controller->setSettings(nullptr);
    }
}

void SpriteEdit::changeEvent(QEvent *event) {
    if(event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}
