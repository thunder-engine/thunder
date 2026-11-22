#include "legacyspritecontroller.h"

#include <components/camera.h>
#include <components/actor.h>
#include <components/transform.h>

#include <viewport/handles.h>
#include <viewport/handletools.h>

#include <input.h>

LegacySpriteController::LegacySpriteController(LegacySpriteEdit *editor) :
        LegacyCameraController(),
        m_settings(nullptr),
        m_spriteTool(new SpriteTool(this)),
        m_editor(editor),
        m_width(0),
        m_height(0),
        m_drag(false) {

}

Object::ObjectList LegacySpriteController::selected() {
    if(m_settings && !m_key.isEmpty()) {
        TextureImportSettings::ElementMap &lements = m_settings->elements();
        auto it = lements.find(m_key);
        if(it != lements.end()) {
            return {it->second};
        }
    }
    return Object::ObjectList();
}

void LegacySpriteController::setSettings(TextureImportSettings *settings) {
    if(m_settings != settings) {
        m_settings = settings;

        static_cast<SpriteTool *>(m_spriteTool)->setSettings(m_settings);

        m_key.clear();
    }
}

void LegacySpriteController::setSize(uint32_t width, uint32_t height) {
    m_width = width;
    m_height = height;
}

bool LegacySpriteController::isSelected(const TString &key) const {
    return (m_key == key);
}

void LegacySpriteController::selectElement(const TString &key) {
    m_key = key;
    selectionChanged();
}

TString LegacySpriteController::selectedElement() {
    return m_key;
}

void LegacySpriteController::setDrag(bool drag) {
    if(drag) {
        m_spriteTool->beginControl();
    }
    m_drag = drag;
}

Vector3 LegacySpriteController::world() const {
    return m_world;
}

void LegacySpriteController::drawHandles() {
    LegacyCameraController::drawHandles();

    Vector4 pos(Input::mousePosition());
    Handles::s_Mouse = Vector2(pos.z, pos.w);
    Handles::s_Screen = m_screenSize;

    m_world = m_activeCamera->unproject(Vector3(pos.z, pos.w, 0.0f));
    m_world.x = CLAMP(m_world.x, 0.0f, m_width);
    m_world.y = CLAMP(m_world.y, 0.0f, m_height);

    m_spriteTool->update(false, true, Input::isKey(Input::KEY_LEFT_CONTROL));
}
