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
        m_controller(new SpriteController(this)),
        m_proxy(new SpriteProxy) {

    ui->setupUi(this);

    m_proxy->setEditor(this);

    m_controller->activateCamera(1, true);
    m_controller->setGridAxis(CameraController::Axis::Z);
    m_controller->blockRotations(true);

    ui->viewport->setController(m_controller);
    ui->viewport->setWorld(m_world);
    ui->viewport->init(); // must be called after all options set
    ui->viewport->setGridEnabled(false);

    connect(m_controller, &SpriteController::objectsSelected, this, &SpriteEdit::objectsSelected);
    connect(m_controller, &SpriteController::updated, this, &SpriteEdit::updated);

    Camera *camera = m_controller->camera();
    if(camera) {
        camera->setOrthographic(true);
    }

    Actor *object = Engine::composeActor<SpriteRender>(gSpriteRender, m_scene);
    m_render = object->getComponent<SpriteRender>();
    m_render->setMaterial(Engine::loadResource<Material>(".embedded/DefaultSprite.shader"));
    m_render->setLayer(2);

    object = Engine::composeActor<SpriteRender>("CheckerBoard", m_scene);
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
    if(!m_settings.empty()) {
        return m_settings.front()->isModified();
    }
    return false;
}

void SpriteEdit::loadAsset(AssetConverterSettings *settings) {
    if(std::find(m_settings.begin(), m_settings.end(), settings) != m_settings.end()) {
        return;
    }

    if(!m_settings.empty()) {
        Object::disconnect(m_settings.front(), _SIGNAL(updated()), m_proxy, _SLOT(onUpdated()));
    }
    AssetEditor::loadAsset(settings);

    if(m_resource) {
        m_resource->unsubscribe(this);
    }

    m_resource = Engine::loadResource(settings->destination());
    if(m_resource == nullptr) {
        return;
    }

    m_resource->subscribe(&SpriteEdit::resourceUpdated, this);

    Sprite *sprite = dynamic_cast<Sprite *>(m_resource);
    if(sprite) {
         m_render->setSprite(sprite);
    } else {
        Texture *texture = dynamic_cast<Texture *>(m_resource);
        if(texture) {
            m_render->setTexture(texture);
        }
    }

    if(m_resource->state() > Resource::Loading) {
        Texture *texture = m_render->texture();
        if(texture) {
            Vector3 size(texture->width(), texture->height(), 0);

            Transform *renderTransform = m_render->transform();
            renderTransform->setScale(size);
            renderTransform->setPosition(size * 0.5f);

            Vector2 scale(size.x / 20.0f, size.y / 20.0f);
            m_checker->materialInstance(0)->setVector2("scale", &scale);

            Transform *checkerTransform = m_checker->transform();
            checkerTransform->setScale(size);
            checkerTransform->setPosition(size * 0.5f);

            m_render->actor()->setEnabled(true);

            m_controller->setSettings(dynamic_cast<TextureImportSettings *>(m_settings.front()));
            m_controller->setSize(size.x, size.y);

            float bottom;
            m_controller->setFocusOn(m_render->actor(), bottom);
        }
    }

    Object::connect(m_settings.front(), _SIGNAL(updated()), m_proxy, _SLOT(onUpdated()));
}

void SpriteEdit::saveAsset(const TString &) {
    m_settings.front()->saveSettings();
}

StringList SpriteEdit::suffixes() const {
    return static_cast<AssetConverter *>(m_converter)->suffixes();
}

void SpriteEdit::onUpdateTemplate() {
    if(!m_settings.empty()) {
        m_converter->convertTexture(static_cast<TextureImportSettings *>(m_settings.front()));
    }
}

void SpriteEdit::resourceUpdated(int state, void *ptr) {
    SpriteEdit *p = static_cast<SpriteEdit *>(ptr);

    switch(state) {
        case Resource::ToBeUpdated:
        case Resource::Ready: {
            Texture *texture = p->m_render->texture();
            Vector3 size(texture->width(), texture->height(), 0);

            Transform *renderTransform = p->m_render->transform();
            renderTransform->setScale(size);
            renderTransform->setPosition(size * 0.5f);

            Vector2 scale(size.x / 20.0f, size.y / 20.0f);
            p->m_checker->materialInstance(0)->setVector2("scale", &scale);

            Transform *checkerTransform = p->m_checker->transform();
            checkerTransform->setScale(size);
            checkerTransform->setPosition(size * 0.5f);

            p->m_render->actor()->setEnabled(true);

            p->m_controller->setSettings(dynamic_cast<TextureImportSettings *>(p->m_settings.front()));
            p->m_controller->setSize(size.x, size.y);
        } break;
        case Resource::ToBeDeleted: {
            p->m_render->actor()->setEnabled(false);
            p->m_resource = nullptr;

            p->m_controller->setSettings(nullptr);
        } break;
        default: break;
    }
}

void SpriteEdit::changeEvent(QEvent *event) {
    if(event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}
