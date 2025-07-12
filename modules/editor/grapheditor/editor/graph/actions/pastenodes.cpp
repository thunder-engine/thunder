#include "pastenodes.h"

#include <float.h>

PasteNodes::PasteNodes(const std::string &data, int x, int y, GraphController *ctrl, const QString &name, QUndoCommand *parent) :
        m_controller(ctrl),
        m_pos(x, y),
        UndoCommand(name, ctrl, parent) {

    m_document.setContent(QByteArray(data.c_str()));
}

void PasteNodes::undo() {
    m_list.reverse();

    auto g = m_controller->graph();
    for(auto &it : m_list) {
        g->nodeDelete(g->node(it));
    }
    m_controller->selectNodes(m_lastSelect);
    emit g->graphUpdated();
}

void PasteNodes::redo() {
    auto g = m_controller->graph();

    Vector2 maxPos(-FLT_MAX, -FLT_MAX);

    m_list.clear();

    AbstractNodeGraph::NodeList nodes;

    QDomElement nodesElement = m_document.firstChildElement();
    QDomElement nodeElement = nodesElement.firstChildElement();
    while(!nodeElement.isNull()) {
        int32_t index = -1;
        const std::string type = nodeElement.attribute("type").toStdString();
        GraphNode *node = g->nodeCreate(type, index);
        if(node) {
            node->fromXml(nodeElement);

            maxPos.x = MAX(maxPos.x, node->position().x);
            maxPos.y = MAX(maxPos.y, node->position().y);

            m_list.push_back(index);
            nodes.push_back(node);
        }

        nodeElement = nodeElement.nextSiblingElement();
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
    emit g->graphUpdated();
}
