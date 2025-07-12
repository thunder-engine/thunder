#ifndef CHANGENODEPROPERTY_H
#define CHANGENODEPROPERTY_H

#include "../graphcontroller.h"

class ChangeNodeProperty : public UndoCommand {
public:
    ChangeNodeProperty(const Object::ObjectList &objects, const std::string &property, const Variant &value, GraphController *ctrl, const QString &name, QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    std::string m_property;

    Variant m_value;

    std::list<uint32_t> m_objects;

    GraphController *m_controller;

};

#endif // CHANGENODEPROPERTY_H
