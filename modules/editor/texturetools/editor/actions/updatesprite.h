#ifndef UPDATESPRITE_H
#define UPDATESPRITE_H

#include <editor/undostack.h>
#include "../spritecontroller.h"

class UpdateSprite : public UndoCommand {
public:
    UpdateSprite(const TextureImportSettings::Element &element, SpriteController *ctrl, UndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    TString m_key;

    TextureImportSettings::Element m_element;

    SpriteController *m_controller;

};

#endif // UPDATESPRITE_H
