#ifndef CHANGENODEPROPERTY_H
#define CHANGENODEPROPERTY_H

#include "../graphcontroller.h"

class ChangeNodeProperty : public UndoCommand {
public:
    ChangeNodeProperty(const Object::ObjectList &objects, const TString &property, const Variant &value, GraphController *ctrl, const QString &name, QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    TString m_property;

    Variant m_value;

    std::list<uint32_t> m_objects;

    GraphController *m_controller;

};

#endif // CHANGENODEPROPERTY_H
