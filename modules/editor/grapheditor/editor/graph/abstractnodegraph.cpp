#include "abstractnodegraph.h"

#include "graphnode.h"

#include <algorithm>

#include <file.h>
#include <pugixml.hpp>

namespace {
    const char *gGraph("graph");
    const char *gNodes("nodes");
    const char *gNode("node");
    const char *gLinks("links");
    const char *gLink("link");

    const char *gType("type");
    const char *gIndex("index");
    const char *gName("name");

    const char *gSender("sender");
    const char *gReceiver("receiver");
    const char *gIn("in");
    const char *gOut("out");
}

AbstractNodeGraph::AbstractNodeGraph() :
        m_version(0) {
}

GraphNode *AbstractNodeGraph::nodeCreate(const TString &path, int &index) {
    return nullptr;
}

void AbstractNodeGraph::nodeDelete(GraphNode *node) {
    auto it = m_nodes.begin();
    while(it != m_nodes.end()) {
        if(*it == node) {
            for(NodePort &item : node->ports()) {
                linkDelete(&item);
            }
            linkDelete(node);

            it = m_nodes.erase(it);
            delete node;
            break;
        } else {
            ++it;
        }
    }
}

AbstractNodeGraph::Link *AbstractNodeGraph::linkCreate(GraphNode *sender, NodePort *oport, GraphNode *receiver, NodePort *iport) {
    bool result = true;
    for(auto &it : m_links) {
        if(it->sender == sender && it->receiver == receiver &&
           it->oport == oport && it->iport == iport) {
            result = false;
            break;
        }
    }
    if(result) {
        linkDelete(iport);

        if((oport && iport && oport->m_call == iport->m_call) ||
           (oport == nullptr && iport == nullptr)) {
            Link *link = new Link;
            link->sender = sender;
            link->receiver = receiver;
            link->oport = oport;
            link->iport = iport;
            link->ptr = nullptr;
            m_links.push_back(link);

            return link;
        }
    }
    return nullptr;
}

void AbstractNodeGraph::linkDelete(NodePort *port) {
    auto it = m_links.begin();
    while(it != m_links.end()) {
        Link *link = *it;
        if(link->oport == port || link->iport == port) {
            GraphNode *first = link->sender;
            GraphNode *second = link->receiver;

            it = m_links.erase(it);
            delete link;
        } else {
            ++it;
        }
    }
}

void AbstractNodeGraph::linkDelete(GraphNode *node) {
    auto it = m_links.begin();
    while(it != m_links.end()) {
        Link *link = *it;
        if(link->sender == node || link->receiver == node) {
            GraphNode *second = link->sender;
            if(link->sender == node) {
                second = link->receiver;
            }
            it = m_links.erase(it);
            delete link;
        } else {
            ++it;
        }
    }
}

void AbstractNodeGraph::linkDelete(Link *link) {
    auto it = m_links.begin();
    while(it != m_links.end()) {
        if(*it == link) {
            GraphNode *first = link->sender;
            GraphNode *second = link->receiver;

            m_links.erase(it);
            delete link;

            return;
        }
        ++it;
    }
}

const AbstractNodeGraph::LinkList AbstractNodeGraph::findLinks(const GraphNode *node) const {
    LinkList result;
    for(const auto it : m_links) {
        if(it->receiver == node || it->sender == node) {
            result.push_back(it);
        }
    }
    return result;
}

const AbstractNodeGraph::LinkList AbstractNodeGraph::findLinks(const NodePort *port) const {
    LinkList result;
    for(const auto it : m_links) {
        if(it->oport == port || it->iport == port) {
            result.push_back(it);
        }
    }
    return result;
}

const AbstractNodeGraph::Link *AbstractNodeGraph::findLink(const GraphNode *node, const NodePort *port) const {
    for(const auto it : m_links) {
        if(it->receiver == node && it->iport == port) {
            return it;
        }
    }
    return nullptr;
}

GraphNode *AbstractNodeGraph::defaultNode() const {
    return nullptr;
}

bool AbstractNodeGraph::isSingleConnection(const NodePort *port) const {
    int count = 0;
    for(const auto it : m_links) {
        if(it->oport == port || it->iport == port) {
            ++count;
        }
    }
    return (count == 1);
}

GraphNode *AbstractNodeGraph::node(int index) const {
    return (index > -1 && index < m_nodes.size()) ? *std::next(m_nodes.begin(), index) : nullptr;
}

AbstractNodeGraph::Link *AbstractNodeGraph::link(int index) const {
    return (index > -1 && index < m_links.size()) ? *std::next(m_links.begin(), index) : nullptr;
}

int AbstractNodeGraph::node(const GraphNode *node) const {
    return std::distance(m_nodes.begin(), std::find(m_nodes.begin(), m_nodes.end(), node));
}

int AbstractNodeGraph::link(const Link *link) const {
    return std::distance(m_links.begin(), std::find(m_links.begin(), m_links.end(), link));
}

void AbstractNodeGraph::load(const TString &path) {
    for(Link *it : m_links) {
        delete it;
    }
    m_links.clear();

    for(GraphNode *it : m_nodes) {
        delete it;
    }
    m_nodes.clear();

    pugi::xml_document doc;
    File file(path);
    if(file.open(File::ReadOnly)) {
        TString content(file.readAll());
        file.close();
        pugi::xml_document doc;
        if(doc.load_string(content.data()).status == pugi::status_ok) {
            pugi::xml_node document = doc.first_child();
            int version = document.attribute("version").as_int();

            blockSignals(true);

            pugi::xml_node root = document.first_child();
            if(root) {
                loadGraph(root);
            }

            blockSignals(false);

            emitSignal(_SIGNAL(graphUpdated()));

            if(version != m_version) {
                save(path);
            }
        }
    }

    emitSignal(_SIGNAL(graphLoaded()));
}

void AbstractNodeGraph::save(const TString &path) {
    pugi::xml_document xml;

    pugi::xml_node document = xml.append_child("document");

    document.append_attribute("version") = m_version;

    saveGraph(document);

    xml.save_file(path.data(), "    ");
}

StringList AbstractNodeGraph::nodeList() const {
    return StringList();
}

void AbstractNodeGraph::loadGraph(const pugi::xml_node &parent) {
    if(std::string(parent.name()) == gGraph) {
        pugi::xml_node nodes = parent.first_child();
        while(nodes) {
            std::string name(nodes.name());
            if(name == gNodes) {
                pugi::xml_node nodeElement = nodes.first_child();
                while(nodeElement) {
                    int32_t index = nodeElement.attribute(gIndex).as_int(-1);
                    TString type = nodeElement.attribute(gType).value();
                    GraphNode *node = nullptr;
                    if(type.isEmpty()) {
                        node = fallbackRoot();
                    } else {
                        node = nodeCreate(type, index);
                    }
                    if(node) {
                        node->fromXml(nodeElement);
                    }

                    nodeElement = nodeElement.next_sibling();
                }
            }

            if(name == gLinks) {
                onNodesLoaded();

                pugi::xml_node linkElement = nodes.first_child();
                while(linkElement) {
                    GraphNode *snd = node(linkElement.attribute(gSender).as_int());
                    GraphNode *rcv = node(linkElement.attribute(gReceiver).as_int());

                    if(snd && rcv) {
                        int index1 = linkElement.attribute(gOut).as_int();
                        NodePort *op = (index1 > -1) ? snd->port(index1) : nullptr;
                        int index2 = linkElement.attribute(gIn).as_int();
                        NodePort *ip = (index2 > -1) ? rcv->port(index2) : nullptr;

                        linkCreate(snd, op, rcv, ip);
                    }

                    linkElement = linkElement.next_sibling();
                }
            }

            nodes = nodes.next_sibling();
        }
    }
}

void AbstractNodeGraph::onNodesLoaded() {

}

GraphNode *AbstractNodeGraph::fallbackRoot() {
    return nullptr;
}

void AbstractNodeGraph::saveGraph(pugi::xml_node &parent) const {
    pugi::xml_node graph = parent.append_child(gGraph);

    pugi::xml_node nodesElement = graph.append_child(gNodes);
    pugi::xml_node linksElement = graph.append_child(gLinks);

    for(auto node : m_nodes) {
        pugi::xml_node nodeElement = node->toXml();

        nodesElement.append_copy(nodeElement);

        saveLinks(node, linksElement);
    }
}

void AbstractNodeGraph::saveLinks(GraphNode *node, pugi::xml_node &parent) const {
    for(auto l : m_links) {
        if(l->sender == node) {
            pugi::xml_node link = parent.append_child(gLink);

            link.append_attribute(gSender) = AbstractNodeGraph::node(l->sender);
            link.append_attribute(gOut) = (l->oport != nullptr) ? l->sender->portPosition(l->oport) : -1;
            link.append_attribute(gReceiver) = AbstractNodeGraph::node(l->receiver);
            link.append_attribute(gIn) = (l->iport != nullptr) ? l->receiver->portPosition(l->iport) : -1;
        }
    }
}

const AbstractNodeGraph::NodeList &AbstractNodeGraph::nodes() const {
    return m_nodes;
}

const AbstractNodeGraph::LinkList &AbstractNodeGraph::links() const {
    return m_links;
}

void AbstractNodeGraph::reportMessage(GraphNode *node, const TString &text) {

}
