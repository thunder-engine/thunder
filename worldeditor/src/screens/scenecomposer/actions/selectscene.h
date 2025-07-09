#ifndef SELECTSCENE_H
#define SELECTSCENE_H

#include "../objectcontroller.h"

class SelectScene : public UndoCommand {
public:
    SelectScene(Scene *scene, ObjectController *ctrl, const QString &name = QObject::tr("Select Scene"), QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    ObjectController *m_controller;

    uint32_t m_object;

};

#endif // SELECTSCENE_H
