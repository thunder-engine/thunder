#ifndef SELECTSPRITE_H
#define SELECTSPRITE_H

#include <editor/undostack.h>

class SpriteController;

class SelectSprite : public UndoCommand {
public:
    SelectSprite(const TString &key, SpriteController *ctrl, UndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    TString m_key;

    SpriteController *m_controller;

};

#endif // SELECTSPRITE_H
