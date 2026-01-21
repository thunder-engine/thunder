#include "selectnodes.h"

SelectNodes::SelectNodes(const std::list<int32_t> &nodes, const std::list<int32_t> &links, GraphController *ctrl, const TString &name, UndoCommand *parent) :
        UndoCommand(name, parent),
        m_controller(ctrl),
        m_nodes(nodes),
        m_links(links) {

}

void SelectNodes::undo() {
    redo();
}

void SelectNodes::redo() {
    AbstractNodeGraph *g = m_controller->graph();

    std::list<int32_t> links = m_controller->selectedLinks();

    std::list<int32_t> nodes;
    for(auto it : m_controller->selected()) {
        GraphNode *node = static_cast<GraphNode *>(it);
        nodes.push_back(g->node(node));
    }

    if(!m_links.empty()) {
        m_controller->selectLinks(m_links);
    } else {
        m_controller->selectNodes(m_nodes);
    }

    m_links = links;
    m_nodes = nodes;
}
