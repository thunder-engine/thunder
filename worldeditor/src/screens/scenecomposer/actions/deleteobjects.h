#ifndef DELETEOBJECTS_H
#define DELETEOBJECTS_H

#include "../objectcontroller.h"

class DeleteObjects : public UndoCommand {
public:
    DeleteObjects(const Object::ObjectList &objects, ObjectController *ctrl, const TString &name = "Delete Actors", UndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

    Object *getRoot(Object *object, uint32_t originRoot) const;

protected:
    VariantList m_dump;

    std::list<uint32_t> m_objects;
    std::unordered_map<uint32_t, uint32_t> m_cloneCache;

    ObjectController *m_controller;

    uint32_t m_prefab;

};

#endif // DELETEOBJECTS_H
