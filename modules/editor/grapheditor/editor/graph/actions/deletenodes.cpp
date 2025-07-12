#include "deletenodes.h"

DeleteNodes::DeleteNodes(const std::list<int32_t> &selection, GraphController *ctrl, const QString &name, QUndoCommand *parent) :
        UndoCommand(name, ctrl, parent),
        m_indices(selection),
        m_controller(ctrl) {

    m_indices.sort(std::greater<int>());
}

void DeleteNodes::undo() {
    m_controller->graph()->loadGraph(m_document.firstChildElement());
    m_controller->selectNodes(m_indices);
}

void DeleteNodes::redo() {
    m_document.clear();

    auto g = m_controller->graph();

    QDomElement graphElement = m_document.createElement("graph");

    QDomElement nodesElement = m_document.createElement("nodes");
    QDomElement linksElement = m_document.createElement("links");

    AbstractNodeGraph::NodeList list;
    for(auto &it : m_indices) {
        GraphNode *node = g->node(it);
        list.push_back(node);
        for(auto link : g->saveLinks(node)) {
            QDomElement linkElement = m_document.createElement("link");

            QVariantMap fields = link.toMap();
            for(auto &key : fields.keys()) {
                linkElement.setAttribute(key, fields.value(key).toString());
            }
            linksElement.appendChild(linkElement);
        }
    }

    for(auto it : list) {
        QDomElement element = it->toXml(m_document);
        nodesElement.appendChild(element);
        g->nodeDelete(it);
    }

    graphElement.appendChild(nodesElement);
    graphElement.appendChild(linksElement);

    m_document.appendChild(graphElement);

    emit g->graphUpdated();
}
