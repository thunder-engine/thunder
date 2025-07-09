#ifndef CHANGEOBJECTPROPERTY_H
#define CHANGEOBJECTPROPERTY_H

#include "../objectcontroller.h"

class ChangeObjectProperty : public UndoCommand {
public:
    ChangeObjectProperty(const Object::ObjectList &objects, const std::string &property, const Variant &value, ObjectController *ctrl, const QString &name, QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    std::string m_property;

    Variant m_value;

    std::list<uint32_t> m_objects;

    ObjectController *m_controller;

};

#endif // CHANGEOBJECTPROPERTY_H
