#include "deletelinksbyport.h"

DeleteLinksByPort::DeleteLinksByPort(int node, int port, GraphController *ctrl, const QString &name, QUndoCommand *parent) :
        UndoCommand(name, ctrl, parent),
        m_controller(ctrl),
        m_node(node),
        m_port(port) {

}

void DeleteLinksByPort::undo() {
    auto g = m_controller->graph();

    for(int i = 0; i < m_links.size(); ++i) {
        Link link = *std::next(m_links.begin(), i);

        GraphNode *snd = g->node(link.sender);
        GraphNode *rcv = g->node(link.receiver);
        if(snd && rcv) {
            NodePort *op = (link.oport > -1) ? snd->port(link.oport) : nullptr;
            NodePort *ip = (link.iport > -1) ? rcv->port(link.iport) : nullptr;

            g->linkCreate(snd, op, rcv, ip);
        }
    }
    g->emitSignal(_SIGNAL(graphUpdated()));
}

void DeleteLinksByPort::redo() {
    auto g = m_controller->graph();

    GraphNode *node = g->node(m_node);
    if(node) {
        m_links.clear();
        if(m_port == -1) {
            for(auto l : g->findLinks(node)) {
                m_links.push_back({
                    g->node(l->sender),
                    (l->oport != nullptr) ? l->sender->portPosition(l->oport) : -1,
                    g->node(l->receiver),
                    (l->iport != nullptr) ? l->receiver->portPosition(l->iport) : -1 });
            }
            g->linkDelete(node);
        } else {
            NodePort *item = node->port(m_port);
            for(auto l : g->findLinks(item)) {
                m_links.push_back({
                    g->node(l->sender),
                    (l->oport != nullptr) ? l->sender->portPosition(l->oport) : -1,
                    g->node(l->receiver),
                    (l->iport != nullptr) ? l->receiver->portPosition(l->iport) : -1 });
            }
            g->linkDelete(item);
        }
        g->emitSignal(_SIGNAL(graphUpdated()));
    }
}
