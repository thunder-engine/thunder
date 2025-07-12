#ifndef SELECTNODES_H
#define SELECTNODES_H

#include "../graphcontroller.h"

class SelectNodes : public UndoCommand {
public:
    SelectNodes(const std::list<int32_t> &selection, GraphController *ctrl, const QString &name = QObject::tr("Select Node"), QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    GraphController *m_controller;

    std::list<int32_t> m_indices;

};

#endif // SELECTNODES_H
