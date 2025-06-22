#ifndef CREATENODE_H
#define CREATENODE_H

#include "../graphcontroller.h"

class CreateNode : public UndoCommand {
public:
    CreateNode(const std::string &type, int x, int y, GraphController *ctrl, int node = -1, int port = -1, bool out = false, const QString &name = QObject::tr("Create Node"), QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    std::list<int32_t> m_list;
    std::string m_type;
    Vector2 m_point;

    GraphController *m_controller;

    int32_t m_node;

    int32_t m_linkIndex;
    int32_t m_fromNode;
    int32_t m_fromPort;

    bool m_out;

};

#endif // CREATENODE_H
