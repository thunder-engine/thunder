#ifndef DESTROYSPRITE_H
#define DESTROYSPRITE_H

#include <editor/undostack.h>
#include "../spritecontroller.h"

class DestroySprite : public UndoCommand {
public:
    DestroySprite(SpriteController *ctrl, UndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    TString m_key;

    TextureImportSettings::Element m_element;

    SpriteController *m_controller;

};

#endif // DESTROYSPRITE_H
