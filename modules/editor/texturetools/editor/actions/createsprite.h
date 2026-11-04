#ifndef CREATESPRITE_H
#define CREATESPRITE_H

#include <editor/undostack.h>
#include "../spritecontroller.h"

class SpriteController;

class CreateSprite : public UndoCommand {
public:
    CreateSprite(const Vector2 &min, const Vector2 &max, SpriteController *ctrl, UndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    TString m_previous;
    TString m_key;
    Vector4 m_bounds;

    SpriteController *m_controller;

};

#endif // CREATESPRITE_H
