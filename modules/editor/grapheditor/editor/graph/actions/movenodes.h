#ifndef MOVENODES_H
#define MOVENODES_H

#include "../graphcontroller.h"

class MoveNodes : public UndoCommand {
public:
    MoveNodes(const std::list<NodeWidget *> &selection, GraphController *ctrl, const QString &name = QObject::tr("Move Node"), QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    GraphController *m_controller;

    std::vector<int32_t> m_indices;
    std::vector<Vector2> m_points;

};

#endif // MOVENODES_H
