#ifndef CHANGEPROPERTY_H
#define CHANGEPROPERTY_H

#include "../widgetcontroller.h"

class ChangeProperty : public UndoCommand {
public:
    ChangeProperty(const Object::ObjectList &objects, const TString &property, const Variant &value, WidgetController *ctrl, const TString &name, UndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    TString m_property;

    Variant m_value;

    std::list<uint32_t> m_objects;

    WidgetController *m_controller;

};

#endif // CHANGEPROPERTY_H
