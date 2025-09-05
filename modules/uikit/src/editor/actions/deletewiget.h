#ifndef DELETEWIGET_H
#define DELETEWIGET_H

#include "../widgetcontroller.h"

class DeleteObject : public UndoCommand {
public:
    DeleteObject(const Object::ObjectList &objects, WidgetController *ctrl, const TString &name = "Delete Widget", UndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    VariantList m_dump;

    std::list<uint32_t> m_objects;

    WidgetController *m_controller;

};

#endif // DELETEWIGET_H
