#include "createlink.h"

CreateLink::CreateLink(int sender, int oport, int receiver, int iport, GraphController *ctrl, const TString &name, UndoCommand *parent) :
        UndoCommand(name, parent),
        m_controller(ctrl),
        m_sender(sender),
        m_oPort(oport),
        m_receiver(receiver),
        m_iPort(iport),
        m_index(-1) {

}

void CreateLink::undo() {
    AbstractNodeGraph *g = m_controller->graph();

    GraphLink *link = g->link(m_index);
    if(link) {
        g->linkDelete(link);

        g->graphUpdated();
    }
}

void CreateLink::redo() {
    AbstractNodeGraph *g = m_controller->graph();

    GraphNode *snd = g->node(m_sender);
    GraphNode *rcv = g->node(m_receiver);
    if(snd && rcv) {
        NodePort *op = (m_oPort > -1) ? snd->port(m_oPort) : nullptr;
        NodePort *ip = (m_iPort > -1) ? rcv->port(m_iPort) : nullptr;

        m_index = g->link(g->linkCreate(snd, op, rcv, ip));

        g->graphUpdated();
    }
}
