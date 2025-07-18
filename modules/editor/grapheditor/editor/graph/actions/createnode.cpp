#include "createnode.h"

CreateNode::CreateNode(const std::string &type, int x, int y, GraphController *ctrl, int node, int port, bool out, const QString &name, QUndoCommand *parent) :
        UndoCommand(name, ctrl, parent),
        m_type(type),
        m_controller(ctrl),
        m_node(-1),
        m_linkIndex(-1),
        m_fromNode(node),
        m_fromPort(port),
        m_out(out),
        m_point(x, y) {

}

void CreateNode::undo() {
    auto g = m_controller->graph();
    g->nodeDelete(g->node(m_node));
    m_controller->selectNodes(m_list);

    g->emitSignal(_SIGNAL(graphUpdated()));
}

void CreateNode::redo() {
    auto g = m_controller->graph();
    GraphNode *node = g->nodeCreate(m_type, m_linkIndex);
    if(node) {
        node->setPosition(m_point);
        m_node = g->node(node);
    }

    if(m_fromNode > -1) {
        GraphNode *fromNode = g->node(m_fromNode);
        NodePort *item = nullptr;
        if(m_fromPort > -1) {
            if(fromNode) {
                int index = 0;
                for(auto &it : fromNode->ports()) {
                    if(it.m_out == m_out) {
                        if(index == m_fromPort) {
                            item = &it;
                            break;
                        } else {
                            index++;
                        }
                    }
                }
            }
        }

        GraphNode *snd = (m_out) ? fromNode : node;
        GraphNode *rcv = (m_out) ? node : fromNode;
        if(snd && rcv) {
            NodePort *sp = (m_fromPort > -1) ? ((m_out) ? item : snd->port(0)) : nullptr;
            NodePort *rp = (m_fromPort > -1) ? ((m_out) ? rcv->port(0) : item) : nullptr;

            AbstractNodeGraph::Link *link = g->linkCreate(snd, sp, rcv, rp);
            m_linkIndex = g->link(link);
        }
    }

    m_list.clear();
    for(auto it : m_controller->selected()) {
        m_list.push_back(g->node(static_cast<GraphNode *>(it)));
    }

    m_controller->selectNodes({m_node});

    g->emitSignal(_SIGNAL(graphUpdated()));
}
