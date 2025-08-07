#ifndef REMOVECOMPONENT_H
#define REMOVECOMPONENT_H

#include "../objectcontroller.h"

class RemoveComponent : public UndoCommand {
public:
    RemoveComponent(const TString &component, ObjectController *ctrl, const TString &name = QObject::tr("Remove Component").toStdString(), UndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    Variant m_dump;

    ObjectController *m_controller;

    uint32_t m_parent;
    uint32_t m_uuid;

    int32_t m_index;

};

#endif // REMOVECOMPONENT_H
