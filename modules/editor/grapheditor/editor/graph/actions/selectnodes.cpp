#include "selectnodes.h"

SelectNodes::SelectNodes(const std::list<int32_t> &selection, GraphController *ctrl, const TString &name, UndoCommand *parent) :
        UndoCommand(name, parent),
        m_controller(ctrl),
        m_indices(selection) {

}

void SelectNodes::undo() {
    redo();
}

void SelectNodes::redo() {
    AbstractNodeGraph *g = m_controller->graph();

    std::list<int32_t> list;
    for(auto it : m_controller->selected()) {
        GraphNode *node = static_cast<GraphNode *>(it);
        list.push_back(g->node(node));
    }

    m_controller->selectNodes(m_indices);
    m_indices = list;
}
