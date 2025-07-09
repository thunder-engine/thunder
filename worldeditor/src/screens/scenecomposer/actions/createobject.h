#ifndef CREATEOBJECT_H
#define CREATEOBJECT_H

#include "../objectcontroller.h"

class CreateObject : public UndoCommand {
public:
    CreateObject(const std::string &type, Scene *scene, ObjectController *ctrl, QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    std::list<uint32_t> m_objects;

    std::string m_type;

    ObjectController *m_controller;

    uint32_t m_scene;

};

#endif // CREATEOBJECT_H
