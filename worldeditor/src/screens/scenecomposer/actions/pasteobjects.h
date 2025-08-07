#ifndef PASTEOBJECTS_H
#define PASTEOBJECTS_H

#include "../objectcontroller.h"

class PasteObjects : public UndoCommand {
public:
    PasteObjects(ObjectController *ctrl, const TString &name = "Paste Object", UndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    VariantList m_data;

    std::unordered_map<uint32_t, uint32_t> m_uuidPairs;

    ObjectController *m_controller;

};

#endif // PASTEOBJECTS_H
