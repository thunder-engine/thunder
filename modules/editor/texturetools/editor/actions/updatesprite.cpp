#include "updatesprite.h"

UpdateSprite::UpdateSprite(const TextureImportSettings::Element &element, SpriteController *ctrl, UndoCommand *group) :
        UndoCommand(QObject::tr("Update Sprite Element").toStdString(), group),
        m_key(ctrl->selectedElement()),
        m_element(element),
        m_controller(ctrl) {
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
