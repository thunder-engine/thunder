#ifndef SELECTNODES_H
#define SELECTNODES_H

#include "../graphcontroller.h"

class SelectNodes : public UndoCommand {
public:
    SelectNodes(const std::list<int32_t> &nodes, const std::list<int32_t> &links, GraphController *ctrl, const TString &name = QObject::tr("Select Node").toStdString(), UndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    std::list<int32_t> m_nodes;
    std::list<int32_t> m_links;

    GraphController *m_controller;

};

#endif // SELECTNODES_H
