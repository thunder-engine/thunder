#include "deletenodes.h"

DeleteNodes::DeleteNodes(const std::list<int32_t> &selection, GraphController *ctrl, const TString &name, UndoCommand *parent) :
        UndoCommand(name, parent),
        m_indices(selection),
        m_controller(ctrl) {

    m_indices.sort(std::greater<int>());
}

void DeleteNodes::undo() {
    m_controller->graph()->loadGraph(m_document.first_child());
    m_controller->selectNodes(m_indices);
}

void DeleteNodes::redo() {
    m_document.reset();

    auto g = m_controller->graph();

    pugi::xml_node graphElement = m_document.append_child("graph");

    pugi::xml_node nodesElement = graphElement.append_child("nodes");
    pugi::xml_node linksElement = graphElement.append_child("links");

    AbstractNodeGraph::NodeList list;
    for(auto &it : m_indices) {
        GraphNode *node = g->node(it);
        list.push_back(node);

        pugi::xml_node nodeElement = nodesElement.append_child("node");
        node->toXml(nodeElement);

        g->saveLinks(node, linksElement);
    }

    for(auto it : list) {
        g->nodeDelete(it);
    }

    g->emitSignal(_SIGNAL(graphUpdated()));
}
