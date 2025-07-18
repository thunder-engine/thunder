#include "pastenodes.h"

#include <float.h>

PasteNodes::PasteNodes(const std::string &data, int x, int y, GraphController *ctrl, const QString &name, QUndoCommand *parent) :
        m_controller(ctrl),
        m_pos(x, y),
        UndoCommand(name, ctrl, parent) {

    m_document.load_string(data.c_str());
}

void PasteNodes::undo() {
    m_list.reverse();

    auto g = m_controller->graph();
    for(auto &it : m_list) {
        g->nodeDelete(g->node(it));
    }
    m_controller->selectNodes(m_lastSelect);

    g->emitSignal(_SIGNAL(graphUpdated()));
}

void PasteNodes::redo() {
    auto g = m_controller->graph();

    Vector2 maxPos(-FLT_MAX, -FLT_MAX);

    m_list.clear();

    AbstractNodeGraph::NodeList nodes;

    pugi::xml_node nodesElement = m_document.first_child();
    pugi::xml_node nodeElement = nodesElement.first_child();
    while(nodeElement) {
        int32_t index = -1;
        const std::string type = nodeElement.attribute("type").value();
        GraphNode *node = g->nodeCreate(type, index);
        if(node) {
            node->fromXml(nodeElement);

            maxPos.x = MAX(maxPos.x, node->position().x);
            maxPos.y = MAX(maxPos.y, node->position().y);

            m_list.push_back(index);
            nodes.push_back(node);
        }

        nodeElement = nodeElement.next_sibling();
    }

    for(auto it : nodes) {
        Vector2 delta(it->position() - maxPos);
        it->setPosition(m_pos + delta);
    }

    m_lastSelect.clear();
    for(auto it : m_controller->selected()) {
        m_lastSelect.push_back(g->node(static_cast<GraphNode *>(it)));
    }

    m_controller->selectNodes(m_list);

    g->emitSignal(_SIGNAL(graphUpdated()));
}
