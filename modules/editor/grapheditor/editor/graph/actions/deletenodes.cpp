#include "deletenodes.h"

DeleteNodes::DeleteNodes(const std::list<int32_t> &selection, GraphController *ctrl, const TString &name, UndoCommand *parent) :
        UndoCommand(name, parent),
        m_indices(selection),
        m_controller(ctrl) {

    m_indices.sort(std::greater<int>());
}

void DeleteNodes::undo() {
    AbstractNodeGraph *g = m_controller->graph();

    std::list<int32_t> list;
    pugi::xml_node element = m_document.first_child();
    while(element) {
        if(std::string(element.name()) == "nodes") {
            pugi::xml_node nodeElement = element.first_child();
            while(nodeElement) {
                int32_t index = g->loadNode(nodeElement);
                if(index > -1) {
                    list.push_back(index);
                }
                nodeElement = nodeElement.next_sibling();
            }
        }

        if(std::string(element.name()) == "links") {
            pugi::xml_node linkElement = element.first_child();
            while(linkElement) {
                g->loadLink(linkElement);
                linkElement = linkElement.next_sibling();
            }
        }

        element = element.next_sibling();
    }

    m_indices = list;
    m_controller->selectNodes(m_indices);
    g->graphUpdated();
}

void DeleteNodes::redo() {
    m_document.reset();

    AbstractNodeGraph *g = m_controller->graph();

    pugi::xml_node nodesElement = m_document.append_child("nodes");
    pugi::xml_node linksElement = m_document.append_child("links");

    AbstractNodeGraph::NodeList list;
    for(auto &it : m_indices) {
        GraphNode *node = g->node(it);
        if(node) {
            list.push_back(node);

            pugi::xml_node nodeElement = nodesElement.append_child("node");
            node->toXml(nodeElement);

            g->saveLinks(node, linksElement);
        }
    }

    GraphNode *defaultNode = g->defaultNode();
    std::list<int32_t> nodeList;
    if(defaultNode) {
        nodeList.push_back(g->node(defaultNode));
    }
    m_controller->selectNodes(nodeList);

    for(auto it : list) {
        g->nodeDelete(it);
    }
    g->graphUpdated();
}
