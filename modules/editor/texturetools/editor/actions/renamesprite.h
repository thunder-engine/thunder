#ifndef RENAMESPRITE_H
#define RENAMESPRITE_H

#include <editor/undostack.h>

class SpriteController;

class RenameSprite : public UndoCommand {
public:
    RenameSprite(const TString &oldKey, const TString &newKey, SpriteController *ctrl, UndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    TString m_newKey;
    TString m_oldKey;

    SpriteController *m_controller;

};

#endif // RENAMESPRITE_H
