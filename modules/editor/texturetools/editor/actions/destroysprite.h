#ifndef DESTROYSPRITE_H
#define DESTROYSPRITE_H

#include <editor/undostack.h>
#include "../legacyspritecontroller.h"

class DestroySprite : public UndoCommand {
public:
    DestroySprite(LegacySpriteController *ctrl, UndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    TString m_key;

    VariantMap m_back;

    LegacySpriteController *m_controller;

};

#endif // DESTROYSPRITE_H
