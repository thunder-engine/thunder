#ifndef SELECTSPRITE_H
#define SELECTSPRITE_H

#include <editor/undostack.h>

class LegacySpriteController;

class SelectSprite : public UndoCommand {
public:
    SelectSprite(const TString &key, LegacySpriteController *ctrl, UndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    TString m_key;

    LegacySpriteController *m_controller;

};

#endif // SELECTSPRITE_H
