#include "deletelinks.h"

DeleteLinks::DeleteLinks(const std::list<int32_t> &links, GraphController *ctrl, const TString &name, UndoCommand *parent) :
        UndoCommand(name, parent),
        m_controller(ctrl),
        m_indices(links) {

}

void DeleteLinks::undo() {
    AbstractNodeGraph *g = m_controller->graph();

    std::list<int32_t> links;
    pugi::xml_node linksElement = m_document.first_child();
    pugi::xml_node linkElement = linksElement.first_child();
    while(linkElement) {
        int index = g->loadLink(linkElement);
        if(index > -1) {
            links.push_back(index);
        }

        linkElement = linkElement.next_sibling();
    }

    m_indices = links;
    m_controller->selectLinks(m_indices);

    g->graphUpdated();
}

void DeleteLinks::redo() {
    m_document.reset();

    AbstractNodeGraph *g = m_controller->graph();

    pugi::xml_node linksElement = m_document.append_child("links");

    m_controller->selectLinks({});

    for(auto it : m_indices) {
        GraphLink *l = g->link(it);
        if(l) {
            g->saveLink(l, linksElement);
            g->linkDelete(l);
        }

    }
    g->graphUpdated();
}
