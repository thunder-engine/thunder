#ifndef CREATESPRITE_H
#define CREATESPRITE_H

#include <editor/undostack.h>
#include "../legacyspritecontroller.h"

class SpriteController;

class CreateSprite : public UndoCommand {
public:
    CreateSprite(const Vector2 &min, const Vector2 &max, LegacySpriteController *ctrl, UndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    TString m_previous;
    TString m_key;
    Vector4 m_bounds;

    LegacySpriteController *m_controller;

};

#endif // CREATESPRITE_H
