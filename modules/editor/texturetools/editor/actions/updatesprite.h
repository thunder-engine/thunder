#ifndef UPDATESPRITE_H
#define UPDATESPRITE_H

#include <editor/undostack.h>
#include "../legacyspritecontroller.h"

class UpdateSprite : public UndoCommand {
public:
    UpdateSprite(const VariantMap &data, LegacySpriteController *ctrl, UndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    TString m_key;

    VariantMap m_data;

    LegacySpriteController *m_controller;

};

#endif // UPDATESPRITE_H
