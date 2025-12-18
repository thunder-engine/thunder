#ifndef CREATECOMPONENT_H
#define CREATECOMPONENT_H

#include "../objectcontroller.h"

class CreateComponent : public UndoCommand {
public:
    CreateComponent(const TString &type, Object *object, ObjectController *ctrl, UndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:

    TString m_type;

    ObjectController *m_controller;

    uint32_t m_object;
    uint32_t m_component;

};

#endif // CREATECOMPONENT_H
