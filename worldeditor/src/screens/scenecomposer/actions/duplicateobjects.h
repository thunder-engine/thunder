#ifndef DUPLICATEOBJECTS_H
#define DUPLICATEOBJECTS_H

#include "../objectcontroller.h"

class DuplicateObjects : public UndoCommand {
public:
    DuplicateObjects(ObjectController *ctrl, const QString &name = QObject::tr("Duplicate Actors"), QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    std::list<uint32_t> m_objects;
    std::list<uint32_t> m_selected;

    VariantList m_dump;

    ObjectController *m_controller;

};

#endif // DUPLICATEOBJECTS_H
