#include "selectsprite.h"

#include "../legacyspritecontroller.h"

SelectSprite::SelectSprite(const TString &key, LegacySpriteController *ctrl, UndoCommand *group) :
        UndoCommand("Select Sprite Elements", group),
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
