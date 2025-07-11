#ifndef SELECTOBJECTS_H
#define SELECTOBJECTS_H

#include "../objectcontroller.h"

class SelectObjects : public UndoCommand {
public:
    SelectObjects(const std::list<uint32_t> &objects, ObjectController *ctrl, const QString &name = QObject::tr("Selection Change"), QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    std::list<uint32_t> m_objects;

    ObjectController *m_controller;
};

#endif // SELECTOBJECTS_H
