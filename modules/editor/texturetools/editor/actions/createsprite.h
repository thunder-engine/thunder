#ifndef CREATESPRITE_H
#define CREATESPRITE_H

#include <editor/undostack.h>
#include "../spritecontroller.h"

class SpriteController;

class CreateSprite : public UndoCommand {
public:
    CreateSprite(const TextureImportSettings::Element &element, SpriteController *ctrl, UndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    TextureImportSettings::Element m_element;

    TString m_uuid;
    TString m_key;

    SpriteController *m_controller;

};

#endif // CREATESPRITE_H
