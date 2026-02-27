#include "selectsprite.h"

#include "../spritecontroller.h"

SelectSprite::SelectSprite(const TString &key, SpriteController *ctrl, UndoCommand *group) :
        UndoCommand(QObject::tr("Select Sprite Elements").toStdString(), group),
        m_key(key),
        m_controller(ctrl) {
}

void SelectSprite::undo() {
    redo();
}

void SelectSprite::redo() {
    TString temp = m_controller->selectedElement();
    m_controller->selectElement(m_key);
    m_key = temp;
}
