#ifndef MOVENODES_H
#define MOVENODES_H

#include "../legacygraphcontroller.h"

class MoveNodes : public UndoCommand {
public:
    MoveNodes(const std::list<NodeWidget *> &selection, LegacyGraphController *ctrl, const TString &name = QObject::tr("Move Node").toStdString(), UndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    LegacyGraphController *m_controller;

    std::vector<int32_t> m_indices;
    std::vector<Vector2> m_points;

};

#endif // MOVENODES_H
