#ifndef RENAMESPRITE_H
#define RENAMESPRITE_H

#include <editor/undostack.h>

class LegacySpriteController;

class RenameSprite : public UndoCommand {
public:
    RenameSprite(const TString &oldKey, const TString &newKey, LegacySpriteController *ctrl, UndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    TString m_newKey;
    TString m_oldKey;

    LegacySpriteController *m_controller;

};

#endif // RENAMESPRITE_H
