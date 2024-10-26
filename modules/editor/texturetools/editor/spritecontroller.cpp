#include "spritecontroller.h"

#include <QCursor>

#include <components/camera.h>
#include <components/actor.h>
#include <components/transform.h>

#include <editor/viewport/handles.h>
#include <editor/viewport/handletools.h>

#include <gizmos.h>
#include <input.h>

SpriteController::SpriteController(QWidget *view) :
        CameraController(),
        m_settings(nullptr),
        m_spriteTool(new SpriteTool(this, m_dummy)),
        m_width(0),
        m_height(0),
        m_drag(false) {

}

void SpriteController::setSize(uint32_t width, uint32_t height) {
    m_width = width;
    m_height = height;

    Camera *cam = camera();
    if(cam) {
        cam->transform()->setPosition(Vector3(m_width * 0.5f, m_height * 0.5f, 1.0f));
        cam->setOrthoSize(m_height);
        cam->setFocal(m_height);
    }
}

bool SpriteController::isSelected(const std::string &key) const {
    return std::find(m_selected.begin(), m_selected.end(), key) != m_selected.end();
}

void SpriteController::selectElements(const std::list<std::string> &list) {
    m_selected = list;

    if(m_selected.empty()) {
        emit selectionChanged(QString());
    } else {
        emit selectionChanged(m_selected.front().c_str());
    }
}

const std::list<std::string> &SpriteController::selectedElements() {
    return m_selected;
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

SelectSprites::SelectSprites(const std::list<std::string> &list, SpriteController *ctrl, const QString &name, QUndoCommand *group) :
    UndoSprite(ctrl, name, group),
    m_list(list) {
}
void SelectSprites::undo() {
    redo();
}
void SelectSprites::redo() {
    std::list<std::string> temp = m_controller->selectedElements();
    m_controller->selectElements(m_list);
    m_list = temp;
}

CreateSprite::CreateSprite(const TextureImportSettings::Element &element, SpriteController *ctrl, QUndoCommand *group) :
    UndoSprite(ctrl, QObject::tr("Create Sprite Element"), group),
    m_element(element) {
}
void CreateSprite::undo() {
    TextureImportSettings *settings = m_controller->settings();
    if(settings) {
        settings->removeElement(m_uuid);
        m_controller->selectElements(m_list);
    }
}
void CreateSprite::redo() {
    TextureImportSettings *settings = m_controller->settings();
    if(settings) {
        m_uuid = settings->setElement(m_element, m_uuid);
        m_list = m_controller->selectedElements();
        m_controller->selectElements({m_uuid});
    }
}

DestroySprites::DestroySprites(SpriteController *ctrl, const QString &name, QUndoCommand *group) :
    UndoSprite(ctrl, name, group),
    m_list(ctrl->selectedElements()) {
}
void DestroySprites::undo() {
    TextureImportSettings *settings = m_controller->settings();
    if(settings) {
        for(int32_t i = 0; i < m_elements.size(); i++) {
            settings->setElement(*std::next(m_elements.begin(), i), *std::next(m_list.begin(), i));
        }
        m_controller->selectElements(m_list);
    }
}
void DestroySprites::redo() {
    TextureImportSettings *settings = m_controller->settings();
    if(settings) {
        m_elements.clear();
        for(auto &it : m_list) {
            auto element = settings->elements().find(it);
            if(element != settings->elements().end()) {
                m_elements.push_back(element->second);
            }
            settings->removeElement(it);
        }
        m_controller->selectElements({});
    }
}

UpdateSprites::UpdateSprites(const std::list<TextureImportSettings::Element> &list, SpriteController *ctrl, const QString &name, QUndoCommand *group) :
        UndoSprite(ctrl, name, group),
        m_list(ctrl->selectedElements()),
        m_elements(list) {
}
void UpdateSprites::undo() {
    redo();
}
void UpdateSprites::redo() {
    TextureImportSettings *settings = m_controller->settings();
    if(settings) {
        std::list<TextureImportSettings::Element> temp;
        for(int32_t i = 0; i < m_elements.size(); i++) {
            auto key = next(m_list.begin(), i);
            auto element = settings->elements().find(*key);
            if(element != settings->elements().end()) {
                TextureImportSettings::Element back = element->second;
                temp.push_back(back);
            }
            settings->setElement(*std::next(m_elements.begin(), i), *std::next(m_list.begin(), i));
        }
        m_elements = temp;
        m_controller->selectElements(m_list);
    }
}
