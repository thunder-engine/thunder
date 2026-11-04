#include "renamesprite.h"

#include "../spritecontroller.h"

RenameSprite::RenameSprite(const TString &oldKey, const TString &newKey, SpriteController *ctrl, UndoCommand *group) :
        UndoCommand(QObject::tr("Rename Sprite Element").toStdString(), group),
        m_oldKey(oldKey),
        m_newKey(newKey),
        m_controller(ctrl) {
}

void RenameSprite::undo() {
    redo();
}

void RenameSprite::redo() {
    TextureImportSettings *settings = m_controller->settings();
    if(settings) {
        Sprite *element = settings->element(m_oldKey);
        if(element) {
            settings->removeElement(m_oldKey);
            element->setName(m_newKey);
            settings->setElement(element, m_newKey);

            m_controller->selectElement(m_newKey);

            std::swap(m_oldKey, m_newKey);
        }
    }
}
