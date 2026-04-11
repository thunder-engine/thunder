#ifndef CREATEOBJECT_H
#define CREATEOBJECT_H

#include "../objectcontroller.h"

class CreateObject : public UndoCommand {
public:
    CreateObject(const TString &type, Object *parent, const Vector3 &position, ObjectController *ctrl);
    void undo() override;
    void redo() override;

protected:
    Object *createObject(Object *parent);

    void resolveUUID(Object::ObjectList &list, Object *root, bool generate);

protected:
    std::list<uint32_t> m_objects;
    std::list<uint32_t> m_selected;
    std::unordered_map<uint32_t, uint32_t> m_staticIds;

    TString m_type;

    Vector3 m_position;

    ObjectController *m_controller;

    uint32_t m_parent;

    uint32_t m_prefab;

};

#endif // CREATEOBJECT_H
