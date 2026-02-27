#include "destroysprite.h"

DestroySprite::DestroySprite(SpriteController *ctrl, UndoCommand *group) :
        UndoCommand(QObject::tr("Destroy Sprite Element").toStdString(), group),
        m_key(ctrl->selectedElement()),
        m_controller(ctrl) {
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
