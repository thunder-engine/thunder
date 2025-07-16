#include "abstractnodegraph.h"

#include "graphnode.h"

#include <QFile>

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
        for(auto &it : m_links) {
            if(it->iport == iport && iport != nullptr) {
                linkDelete(iport);
            }
        }

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

GraphNode *AbstractNodeGraph::node(int index) {
    return (index > -1 && index < m_nodes.size()) ? *std::next(m_nodes.begin(), index) : nullptr;
}

AbstractNodeGraph::Link *AbstractNodeGraph::link(int index) {
    return (index > -1 && index < m_links.size()) ? *std::next(m_links.begin(), index) : nullptr;
}

int AbstractNodeGraph::node(GraphNode *node) const {
    return std::distance(m_nodes.begin(), std::find(m_nodes.begin(), m_nodes.end(), node));
}

int AbstractNodeGraph::link(Link *link) const {
    return std::distance(m_links.begin(), std::find(m_links.begin(), m_links.end(), link));
}

void AbstractNodeGraph::load(const String &path) {
    for(Link *it : m_links) {
        delete it;
    }
    m_links.clear();

    for(GraphNode *it : m_nodes) {
        delete it;
    }
    m_nodes.clear();

    QFile loadFile(path.data());
    if(!loadFile.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open file.");
        return;
    }
    QByteArray data(loadFile.readAll());
    loadFile.close();

    QDomDocument doc;
    doc.setContent(data);

    QDomElement document = doc.documentElement();
    int version = document.attribute("version", "0").toInt();

    blockSignals(true);

    QDomNode p = document.firstChild();
    while(!p.isNull()) {
        QDomElement element = p.toElement();
        if(!element.isNull()) {
            loadGraph(element);
        }

        p = p.nextSiblingElement();
    }

    blockSignals(false);

    emit graphUpdated();

    if(version != m_version) {
        save(path);
    }

    emit graphLoaded();
}

void AbstractNodeGraph::save(const String &path) {
    QDomDocument xml;

    QDomElement document = xml.createElement("document");

    document.setAttribute("version", m_version);

    saveGraph(document, xml);

    xml.appendChild(document);

    QFile saveFile(path.data());
    if(saveFile.open(QIODevice::WriteOnly)) {
        saveFile.write(xml.toByteArray(4));
        saveFile.close();
    }
}

StringList AbstractNodeGraph::nodeList() const {
    return StringList();
}

void AbstractNodeGraph::loadGraph(const QDomElement &parent) {
    if(parent.tagName() == gGraph) {
        QDomElement nodes = parent.firstChildElement(gNodes);
        if(!nodes.isNull()) {
            QDomElement nodeElement = nodes.firstChildElement();
            while(!nodeElement.isNull()) {
                int32_t index = nodeElement.attribute(gIndex, "-1").toInt();
                String type = nodeElement.attribute(gType).toStdString();
                GraphNode *node = nullptr;
                if(type.isEmpty()) {
                    node = fallbackRoot();
                } else {
                    node = nodeCreate(type, index);
                }
                if(node) {
                    node->fromXml(nodeElement);
                }

                nodeElement = nodeElement.nextSiblingElement();
            }
        }

        onNodesLoaded();

        QDomElement links = parent.firstChildElement(gLinks);
        if(!links.isNull()) {
            QDomElement linkElement = links.firstChildElement();
            while(!linkElement.isNull()) {
                GraphNode *snd = node(linkElement.attribute(gSender).toInt());
                GraphNode *rcv = node(linkElement.attribute(gReceiver).toInt());

                if(snd && rcv) {
                    int index1 = linkElement.attribute(gOut).toInt();
                    NodePort *op = (index1 > -1) ? snd->port(index1) : nullptr;
                    int index2 = linkElement.attribute(gIn).toInt();
                    NodePort *ip = (index2 > -1) ? rcv->port(index2) : nullptr;

                    linkCreate(snd, op, rcv, ip);
                }

                linkElement = linkElement.nextSiblingElement();
            }
        }
    }
}

void AbstractNodeGraph::onNodesLoaded() {

}

GraphNode *AbstractNodeGraph::fallbackRoot() {
    return nullptr;
}

void AbstractNodeGraph::saveGraph(QDomElement &parent, QDomDocument &xml) const {
    QDomElement graph = xml.createElement(gGraph);

    QVariantList links;

    QDomElement nodesElement = xml.createElement(gNodes);
    QDomElement linksElement = xml.createElement(gLinks);

    for(auto node : m_nodes) {
        QDomElement nodeElement = node->toXml(xml);

        nodesElement.appendChild(nodeElement);

        for(auto link : saveLinks(node)) {
            QDomElement linkElement = xml.createElement(gLink);

            QVariantMap fields = link.toMap();
            for(auto &key : fields.keys()) {
                linkElement.setAttribute(key, fields.value(key).toString());
            }
            linksElement.appendChild(linkElement);
        }
    }

    graph.appendChild(nodesElement);
    graph.appendChild(linksElement);

    parent.appendChild(graph);
}

QVariantList AbstractNodeGraph::saveLinks(GraphNode *node) const {
    QVariantList result;

    for(auto l : m_links) {
        if(l->sender == node) {
            QVariantMap link;
            link[gSender] = AbstractNodeGraph::node(l->sender);
            link[gOut] = (l->oport != nullptr) ? l->sender->portPosition(l->oport) : -1;
            link[gReceiver] = AbstractNodeGraph::node(l->receiver);
            link[gIn] = (l->iport != nullptr) ? l->receiver->portPosition(l->iport) : -1;

            result.push_back(link);
        }
    }

    return result;
}

const AbstractNodeGraph::NodeList &AbstractNodeGraph::nodes() const {
    return m_nodes;
}

const AbstractNodeGraph::LinkList &AbstractNodeGraph::links() const {
    return m_links;
}

void AbstractNodeGraph::reportMessage(GraphNode *node, const String &text) {
    emit messageReported(AbstractNodeGraph::node(node), text);
}
