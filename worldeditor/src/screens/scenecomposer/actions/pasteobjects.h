#ifndef PASTEOBJECTS_H
#define PASTEOBJECTS_H

#include "../objectcontroller.h"

class PasteObjects : public UndoCommand {
public:
    PasteObjects(const VariantList &data, Object *parent, ObjectController *ctrl, const TString &name = "Paste Object", UndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

    Object *buildObject(const Variant &data, Object *parent, bool generate);

    void detachObjectUUID(Object *object, uint32_t uuid);

protected:
    VariantList m_data;

    std::unordered_map<uint32_t, uint32_t> m_uuidPairs;

    std::list<uint32_t> m_selected;

    ObjectController *m_controller;

    uint32_t m_parent;

    uint32_t m_prefab;

};

#endif // PASTEOBJECTS_H
