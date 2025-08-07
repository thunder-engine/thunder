#ifndef DELETEOBJECTS_H
#define DELETEOBJECTS_H

#include "../objectcontroller.h"

class DeleteObjects : public UndoCommand {
public:
    DeleteObjects(const Object::ObjectList &objects, ObjectController *ctrl, const TString &name = QObject::tr("Delete Actors").toStdString(), UndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    VariantList m_dump;

    std::list<uint32_t> m_parents;
    std::list<uint32_t> m_objects;
    std::list<uint32_t> m_indices;

    ObjectController *m_controller;

};

#endif // DELETEOBJECTS_H
