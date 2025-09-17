#ifndef CREATEOBJECTSERIAL_H
#define CREATEOBJECTSERIAL_H

#include "../objectcontroller.h"

class CreateObjectSerial : public UndoCommand {
public:
    CreateObjectSerial(const TString &ref, const Vector3 &position, uint32_t parent, ObjectController *ctrl, const TString &name = QObject::tr("Create Actors").toStdString(), UndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    std::list<uint32_t> m_selectedObjects;
    std::unordered_map<uint32_t, uint32_t> m_staticIds;

    TString m_reference;

    Vector3 m_position;

    ObjectController *m_controller;

    uint32_t m_parent;

};

#endif // CREATEOBJECTSERIAL_H
