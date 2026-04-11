#ifndef PARENTOBJECTS_H
#define PARENTOBJECTS_H

#include "../objectcontroller.h"

class ParentObjects : public UndoCommand {
public:
    ParentObjects(const Object::ObjectList &objects, Object *parent, int32_t position, ObjectController *ctrl, const TString &name = "Parent Change", UndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    std::unordered_map<uint32_t, uint32_t> m_parentCache;
    std::list<uint32_t> m_objects;

    ObjectController *m_controller;

    uint32_t m_prefab;

    uint32_t m_parent;

    int32_t m_position;

};

#endif // PARENTOBJECTS_H
