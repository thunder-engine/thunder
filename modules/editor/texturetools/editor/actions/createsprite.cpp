#include "createsprite.h"

CreateSprite::CreateSprite(const TextureImportSettings::Element &element, SpriteController *ctrl, UndoCommand *group) :
        UndoCommand(QObject::tr("Create Sprite Element").toStdString(), group),
        m_element(element),
        m_controller(ctrl) {
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
