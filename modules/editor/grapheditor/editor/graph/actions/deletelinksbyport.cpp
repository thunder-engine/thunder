#include "deletelinksbyport.h"

DeleteLinksByPort::DeleteLinksByPort(int node, int port, GraphController *ctrl, const TString &name, UndoCommand *parent) :
        UndoCommand(name, parent),
        m_controller(ctrl),
        m_node(node),
        m_port(port) {

}

void DeleteLinksByPort::undo() {
    AbstractNodeGraph *g = m_controller->graph();

    for(Link &link : m_links) {
        GraphNode *snd = g->node(link.sender);
        GraphNode *rcv = g->node(link.receiver);
        if(snd && rcv) {
            NodePort *op = (link.oport > -1) ? snd->port(link.oport) : nullptr;
            NodePort *ip = (link.iport > -1) ? rcv->port(link.iport) : nullptr;

            g->linkCreate(snd, op, rcv, ip);
        }
    }
    g->graphUpdated();
}

void DeleteLinksByPort::redo() {
    AbstractNodeGraph *g = m_controller->graph();

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
        g->graphUpdated();
    }
}
