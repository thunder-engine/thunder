#ifndef CREATEOBJECTSERIAL_H
#define CREATEOBJECTSERIAL_H

#include "../objectcontroller.h"

class CreateObjectSerial : public UndoCommand {
public:
    CreateObjectSerial(Object::ObjectList &list, ObjectController *ctrl, const QString &name = QObject::tr("Create Actors"), QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    VariantList m_dump;

    std::list<uint32_t> m_parents;
    std::list<uint32_t> m_objects;

    ObjectController *m_controller;

};

#endif // CREATEOBJECTSERIAL_H
