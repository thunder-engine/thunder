#include "spritecontroller.h"

#include <QCursor>

#include <components/camera.h>
#include <components/actor.h>
#include <components/transform.h>

#include <editor/viewport/handles.h>
#include <editor/viewport/handletools.h>

#include <input.h>

SpriteController::SpriteController() :
        CameraController(),
        m_settings(nullptr),
        m_spriteTool(new SpriteTool(this)),
        m_width(0),
        m_height(0),
        m_drag(false) {

}

void SpriteController::setSettings(TextureImportSettings *settings) {
    m_settings = settings;

    static_cast<SpriteTool *>(m_spriteTool)->setSettings(m_settings);

    m_key.clear();
}

void SpriteController::setSize(uint32_t width, uint32_t height) {
    m_width = width;
    m_height = height;

    Camera *cam = camera();
    if(cam) {
        cam->transform()->setPosition(Vector3(m_width * 0.5f, m_height * 0.5f, 1.0f));
        cam->setOrthoSize(MAX(m_width, m_height));
        cam->setFocal(m_height);
    }
}

bool SpriteController::isSelected(const std::string &key) const {
    return (m_key == key);
}

void SpriteController::selectElement(const std::string &key) {
    m_key = key;
}

std::string SpriteController::selectedElement() {
    return m_key;
}

void SpriteController::setDrag(bool drag) {
    if(drag) {
        m_spriteTool->beginControl();
    }
    m_drag = drag;
}

Vector3 SpriteController::world() const {
    return m_world;
}

void SpriteController::update() {
    CameraController::update();

    if(m_settings == nullptr) {
        return;
    }

    if(m_spriteTool->cursor() != Qt::ArrowCursor) {
        emit setCursor(QCursor(m_spriteTool->cursor()));
    } else {
        emit unsetCursor();
    }
}

void SpriteController::drawHandles() {
    CameraController::drawHandles();

    Vector4 pos(Input::mousePosition());
    Handles::s_Mouse = Vector2(pos.z, pos.w);
    Handles::s_Screen = m_screenSize;

    m_world = m_activeCamera->unproject(Vector3(pos.z, pos.w, 0.0f));
    m_world.x = CLAMP(m_world.x, 0.0f, m_width);
    m_world.y = CLAMP(m_world.y, 0.0f, m_height);

    m_spriteTool->update(false, true, Input::isKey(Input::KEY_LEFT_CONTROL));
}

SelectSprite::SelectSprite(const std::string &key, SpriteController *ctrl, const QString &name, QUndoCommand *group) :
        UndoSprite(ctrl, name, group),
        m_key(key) {
}
void SelectSprite::undo() {
    redo();
}
void SelectSprite::redo() {
    std::string temp = m_controller->selectedElement();
    m_controller->selectElement(m_key);
    m_key = temp;
}

CreateSprite::CreateSprite(const TextureImportSettings::Element &element, SpriteController *ctrl, QUndoCommand *group) :
    UndoSprite(ctrl, QObject::tr("Create Sprite Element"), group),
    m_element(element) {
}
void CreateSprite::undo() {
    TextureImportSettings *settings = m_controller->settings();
    if(settings) {
        settings->removeElement(m_uuid);
        m_controller->selectElement(m_key);
    }
}
void CreateSprite::redo() {
    TextureImportSettings *settings = m_controller->settings();
    if(settings) {
        m_uuid = settings->setElement(m_element, "");
        m_key = m_controller->selectedElement();
        m_controller->selectElement({m_uuid});
    }
}

DestroySprite::DestroySprite(SpriteController *ctrl, const QString &name, QUndoCommand *group) :
        UndoSprite(ctrl, name, group),
        m_key(ctrl->selectedElement()) {
}
void DestroySprite::undo() {
    TextureImportSettings *settings = m_controller->settings();
    if(settings) {
        settings->setElement(m_element, m_key);

        m_controller->selectElement(m_key);
    }
}
void DestroySprite::redo() {
    TextureImportSettings *settings = m_controller->settings();
    if(settings) {
        auto element = settings->elements().find(m_key);
        if(element != settings->elements().end()) {
            m_element = element->second;
        }
        settings->removeElement(m_key);

        m_controller->selectElement({});
    }
}

UpdateSprite::UpdateSprite(const TextureImportSettings::Element &element, SpriteController *ctrl, const QString &name, QUndoCommand *group) :
        UndoSprite(ctrl, name, group),
        m_key(ctrl->selectedElement()),
        m_element(element) {
}
void UpdateSprite::undo() {
    redo();
}
void UpdateSprite::redo() {
    TextureImportSettings *settings = m_controller->settings();
    if(settings) {
        auto element = settings->elements().find(m_key);
        if(element != settings->elements().end()) {
            TextureImportSettings::Element temp = element->second;
            settings->setElement(m_element, m_key);
            m_element = temp;

            m_controller->selectElement(m_key);
            m_controller->updated();
        }
    }
}

RenameSprite::RenameSprite(const std::string &oldKey, const std::string &newKey, SpriteController *ctrl, const QString &name, QUndoCommand *group) :
        UndoSprite(ctrl, name, group),
        m_oldKey(oldKey),
        m_newKey(newKey) {
}
void RenameSprite::undo() {
    redo();
}
void RenameSprite::redo() {
    TextureImportSettings *settings = m_controller->settings();
    if(settings) {
        auto element = settings->elements().find(m_oldKey);
        if(element != settings->elements().end()) {
            TextureImportSettings::Element temp = element->second;
            settings->removeElement(m_oldKey);
            settings->setElement(temp, m_newKey);

            m_controller->selectElement(m_newKey);

            std::swap(m_oldKey, m_newKey);
        }
    }
}
