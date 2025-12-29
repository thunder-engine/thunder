#include "movenodes.h"

#include "../graphwidgets/nodewidget.h"

#include <components/recttransform.h>

MoveNodes::MoveNodes(const std::list<NodeWidget *> &selection, GraphController *ctrl, const TString &name, UndoCommand *parent) :
        UndoCommand(name, parent),
        m_controller(ctrl) {

    AbstractNodeGraph *g = m_controller->graph();

    m_indices.reserve(selection.size());
    m_points.reserve(selection.size());
    for(auto &it : selection) {
        m_indices.push_back(g->node(it->node()));

        RectTransform *rect = it->rectTransform();
        Vector3 pos(rect->position());
        m_points.push_back(Vector2(pos.x, pos.y));
    }
}

void MoveNodes::undo() {
    redo();
}

void MoveNodes::redo() {
    AbstractNodeGraph *g = m_controller->graph();

    std::vector<Vector2> positions(m_indices.size());
    for(int i = 0; i < m_indices.size(); i++) {
        GraphNode *node = g->node(m_indices.at(i));
        positions[i] = node->position();
        node->setPosition(m_points.at(i));

        // Update widget position
        RectTransform *rect = static_cast<NodeWidget *>(node->widget())->rectTransform();
        rect->setPosition(Vector3(node->position(), 0.0f));
    }
    // Recalc links positions
    static_cast<GraphController *>(m_controller)->composeLinks();
    m_points = positions;
}
