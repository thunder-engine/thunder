#ifndef CREATEOBJECT_H
#define CREATEOBJECT_H

#include "../objectcontroller.h"

class CreateObject : public UndoCommand {
public:
    CreateObject(const TString &type, Scene *scene, ObjectController *ctrl, UndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    std::list<uint32_t> m_objects;
    std::list<uint32_t> m_selected;

    TString m_type;

    ObjectController *m_controller;

    uint32_t m_scene;

};

#endif // CREATEOBJECT_H
