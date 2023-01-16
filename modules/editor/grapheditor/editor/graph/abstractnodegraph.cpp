#include "abstractnodegraph.h"

#include "graphnode.h"

#include <QJsonArray>
#include <QFile>
#include <QGuiApplication>
#include <QClipboard>

namespace {
    const char *gNodes("Nodes");
    const char *gLinks("Links");
    const char *gValues("Values");

    const char *gNode("Node");
    const char *gType("Type");
    const char *gName("Name");
    const char *gLink("Link");
    const char *gValue("Value");
    const char *gIndex("Index");

    const char *gSender("Sender");
    const char *gReceiver("Receiver");
    const char *gIPort("IPort");
    const char *gOPort("OPort");

    const char *gX("X");
    const char *gY("Y");
}

AbstractNodeGraph::AbstractNodeGraph() :
        m_rootNode(nullptr) {
}

GraphNode *AbstractNodeGraph::rootNode() const {
    return m_rootNode;
}

void AbstractNodeGraph::nodeDelete(GraphNode *node) {
    if(node == m_rootNode) {
        return;
    }
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

        Link *link = new Link;
        link->sender = sender;
        link->receiver = receiver;
        link->oport = oport;
        link->iport = iport;
        link->ptr = nullptr;
        m_links.push_back(link);

        return link;
    }
    return nullptr;
}

void AbstractNodeGraph::linkDelete(NodePort *port) {
    auto it = m_links.begin();
    while(it != m_links.end()) {
        Link *link = *it;
        if(link->oport == port || link->iport == port) {
            it  = m_links.erase(it);
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
    return (index > -1) ? m_nodes.at(index) : nullptr;
}

AbstractNodeGraph::Link *AbstractNodeGraph::link(int index) {
    return (index > -1) ? m_links.at(index) : nullptr;
}

int AbstractNodeGraph::node(GraphNode *node) const {
    return m_nodes.indexOf(node);
}

int AbstractNodeGraph::link(Link *link) const {
    return m_links.indexOf(link);
}

void AbstractNodeGraph::load(const QString &path) {
    for(Link *it : qAsConst(m_links)) {
        delete it;
    }
    m_links.clear();

    for(GraphNode *it : qAsConst(m_nodes)) {
        delete it;
    }
    m_nodes.clear();

    m_rootNode = createRoot();
    m_nodes.push_back(m_rootNode);

    QFile loadFile(path);
    if(!loadFile.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open file.");
        return;
    }
    QByteArray data(loadFile.readAll());
    loadFile.close();

    if(data.at(0) == '{') { // Load old json format
        m_data = QJsonDocument::fromJson(data).toVariant().toMap();
        loadGraph(m_data);
        save(path); // Need to save in new xml format
    } else if(data.at(0) == '<') { // Load new xml format
        m_data = loadXmlData(data);
        loadGraph(m_data);
    }
}

void AbstractNodeGraph::save(const QString &path) {
    QVariantList nodes;
    QVariantList links;

    for(GraphNode *it : qAsConst(m_nodes)) {
        if(it != m_rootNode) {
            nodes.push_back(saveNode(it));
        }
        links.append(saveLinks(it));
    }
    m_data[gNodes] = nodes;
    m_data[gLinks] = links;

    QFile saveFile(path);
    if(saveFile.open(QIODevice::WriteOnly)) {
        saveFile.write(saveXmlData(m_data));
        saveFile.close();
    }
}

QStringList AbstractNodeGraph::nodeList() const {
    return QStringList();
}

QVariantMap AbstractNodeGraph::loadXmlData(const QByteArray &data) {
    QDomDocument doc;
    doc.setContent(data);

    return loadXmlMap(doc.documentElement());
}

QVariantMap AbstractNodeGraph::loadXmlMap(const QDomElement &parent) {
    QVariantMap result;

    QDomNode n = parent.firstChild();
    while(!n.isNull()) {
        QDomElement e = n.toElement();
        if(!e.isNull()) {
            switch(e.attribute(gType).toInt()) {
                case QMetaType::Bool: {
                    bool value = (e.text() == "true");
                    result[e.attribute(gName)] = value;
                } break;
                case QMetaType::Int: {
                    int value = e.text().toInt();
                    result[e.attribute(gName)] = value;
                } break;
                case QMetaType::Float:
                case QMetaType::Double: {
                    float value = e.text().toFloat();
                    result[e.attribute(gName)] = value;
                } break;
                case QMetaType::QString: {
                    QString value = e.text();
                    result[e.attribute(gName)] = value;
                } break;
                case QVariant::Map: {
                    result[e.attribute(gName)] = loadXmlMap(e);
                } break;
                case QVariant::List: {
                    result[e.attribute(gName)] = loadXmlList(e);
                } break;
                default: break;
            }
        }
        n = n.nextSibling();
    }

    return result;
}

QVariantList AbstractNodeGraph::loadXmlList(const QDomElement &parent) {
    QVariantList result;

    QDomNode n = parent.firstChild();
    while(!n.isNull()) {
        QDomElement e = n.toElement();
        if(!e.isNull()) {
            switch(e.attribute(gType).toInt()) {
                case QMetaType::Bool: {
                    bool value = (e.text() == "true");
                    result.push_back(value);
                } break;
                case QMetaType::Int: {
                    int value = e.text().toInt();
                    result.push_back(value);
                } break;
                case QMetaType::Float:
                case QMetaType::Double: {
                    float value = e.text().toFloat();
                    result.push_back(value);
                } break;
                case QMetaType::QString: {
                    QString value = e.text();
                    result.push_back(value);
                } break;
                case QMetaType::QVariantMap: {
                    result.push_back(loadXmlMap(e));
                } break;
                case QMetaType::QVariantList: {
                    result.push_back(loadXmlList(e));
                } break;
                default: break;
            }
        }
        n = n.nextSibling();
    }

    return result;
}

QByteArray AbstractNodeGraph::saveXmlData(const QVariantMap &data) {
    QDomDocument xml;
    QDomElement graph = xml.createElement("Graph");
    saveXmlMap(data, xml, graph);
    xml.appendChild(graph);

    return xml.toByteArray(4);
}

void AbstractNodeGraph::saveXmlMap(const QVariantMap &data, QDomDocument &xml, QDomElement &parent) {
    for(auto &it : data.keys()) {
        QDomElement value = xml.createElement(gValue);
        value.setAttribute(gName, it);
        QVariant variant = data.value(it);
        value.setAttribute(gType, variant.type());
        if(variant.type() == QVariant::List) {
            saveXmlList(variant.toList(), xml, value);
        } else if(variant.type() == QVariant::Map) {
            saveXmlMap(variant.toMap(), xml, value);
        } else {
            QString str = variant.toString();
            if(variant.type() == QVariant::String) {
                value.appendChild(xml.createCDATASection(str));
            } else {
                value.appendChild(xml.createTextNode(str));
            }
        }
        parent.appendChild(value);
    }
}

void AbstractNodeGraph::saveXmlList(const QVariantList &data, QDomDocument &xml, QDomElement &parent) {
    for(auto &it : data) {
        QDomElement value = xml.createElement(gValue);
        value.setAttribute(gType, it.type());
        if(it.type() == QVariant::List) {
            saveXmlList(it.toList(), xml, value);
        } else if(it.type() == QVariant::Map) {
            saveXmlMap(it.toMap(), xml, value);
        } else {
            value.appendChild( xml.createTextNode(it.toString()) );
        }
        parent.appendChild(value);
    }
}

void AbstractNodeGraph::loadGraph(const QVariantMap &data) {
    QVariantList nodes = data[gNodes].toList();
    for(int i = 0; i < nodes.size(); ++i) {
        QVariantMap n = nodes[i].toMap();
        int32_t index = n[gIndex].isValid() ? n[gIndex].toInt() : -1;
        GraphNode *node = nodeCreate(n[gType].toString(), index);
        node->setPosition(Vector2(n[gX].toInt(), n[gY].toInt()));
        loadUserValues(node, n[gValues].toMap());
    }

    QVariantList links = data[gLinks].toList();
    for(int i = 0; i < links.size(); ++i) {
        QVariantMap l = links[i].toMap();

        GraphNode *snd = node(l[gSender].toInt());
        GraphNode *rcv = node(l[gReceiver].toInt());
        if(snd && rcv) {
            int index1 = l[gOPort].toInt();
            NodePort *op = (index1 > -1) ? snd->port(index1) : nullptr;
            int index2 = l[gIPort].toInt();
            NodePort *ip = (index2 > -1) ? rcv->port(index2) : nullptr;

            linkCreate(snd, op, rcv, ip);
        }
    }

    emit graphUpdated();
}

QVariantMap AbstractNodeGraph::saveNode(GraphNode *node) {
    QVariantMap result;
    result[gType] = node->type().c_str();
    result[gX] = (int)node->position().x;
    result[gY] = (int)node->position().y;
    result[gIndex] = AbstractNodeGraph::node(node);

    QVariantMap values;
    saveUserValues(node, values);
    result[gValues] = values;

    return result;
}

QVariantList AbstractNodeGraph::saveLinks(GraphNode *node) {
    QVariantList result;

    for(auto l : findLinks(node)) {
        QVariantMap link;
        link[gSender] = AbstractNodeGraph::node(l->sender);
        link[gOPort] = (l->oport != nullptr) ? l->sender->portPosition(l->oport) : -1;
        link[gReceiver] = AbstractNodeGraph::node(l->receiver);
        link[gIPort] = (l->iport != nullptr) ? l->receiver->portPosition(l->iport) : -1;

        result.push_back(link);
    }

    return result;
}

void AbstractNodeGraph::createNode(const QString &path, int x, int y) {
    UndoManager::instance()->push(new CreateNode(path, x, y, this));
}

void AbstractNodeGraph::createAndLink(const QString &path, int x, int y, int node, int port, bool out) {
    UndoManager::instance()->push(new CreateNode(path, x, y, this, node, port, out));
}

const AbstractNodeGraph::NodeList &AbstractNodeGraph::nodes() const {
    return m_nodes;
}

const AbstractNodeGraph::LinkList &AbstractNodeGraph::links() const {
    return m_links;
}

void AbstractNodeGraph::reportMessage(GraphNode *node, const QString &text) {
    emit messageReported(AbstractNodeGraph::node(node), text);
}

void AbstractNodeGraph::createLink(int sender, int oport, int receiver, int iport) {
    UndoManager::instance()->push(new CreateLink(sender, oport, receiver, iport, this));
}

void AbstractNodeGraph::deleteLink(int index) {
    UndoManager::instance()->push(new DeleteLink(index, this));
}

void AbstractNodeGraph::deleteLinksByPort(int node, int port) {
    UndoManager::instance()->push(new DeleteLinksByPort(node, port, this));
}

void AbstractNodeGraph::deleteNodes(const vector<int32_t> &selection) {
    UndoManager::instance()->push(new DeleteNodes(selection, this));
}

void AbstractNodeGraph::copyNodes(const vector<int32_t> &selection) {
    QJsonArray array;
    for(auto it : selection) {
        GraphNode *node = m_nodes.at(it);

        if(node) {
            array.push_back(QJsonValue::fromVariant(saveNode(node)));
        }
    }

    QGuiApplication::clipboard()->setText(QJsonDocument(array).toJson());
}

QVariant AbstractNodeGraph::pasteNodes(int x, int y) {
    PasteNodes *paste = new PasteNodes(QGuiApplication::clipboard()->text(), x, y, this);
    UndoManager::instance()->push(paste);
    return paste->list();
}

CreateNode::CreateNode(const QString &path, int x, int y, AbstractNodeGraph *graph, int node, int port, bool out, const QString &name, QUndoCommand *parent) :
        UndoGraph(graph, name, parent),
        m_node(nullptr),
        m_path(path),
        m_linkIndex(-1),
        m_fromNode(node),
        m_fromPort(port),
        m_out(out),
        m_point(x, y) {

}
void CreateNode::undo() {
    m_graph->nodeDelete(m_node);
    emit m_graph->graphUpdated();
}
void CreateNode::redo() {
    m_node = m_graph->nodeCreate(m_path, m_linkIndex);
    if(m_node) {
        m_node->setPosition(m_point);
    }

    if(m_fromNode > -1) {
        GraphNode *node = m_graph->node(m_fromNode);
        NodePort *item = nullptr;
        if(m_fromPort > -1) {
            if(node) {
                int index = 0;
                for(auto &it : node->ports()) {
                    if(it.m_out == m_out) {
                        if(index == m_fromPort) {
                            item = &it;
                            break;
                        } else {
                            index++;
                        }
                    }
                }
            }
        }

        GraphNode *snd = (m_out) ? node : m_node;
        GraphNode *rcv = (m_out) ? m_node : node;
        if(snd && rcv) {
            NodePort *sp = (m_fromPort > -1) ? ((m_out) ? item : snd->port(0)) : nullptr;
            NodePort *rp = (m_fromPort > -1) ? ((m_out) ? rcv->port(0) : item) : nullptr;

            AbstractNodeGraph::Link *link = m_graph->linkCreate(snd, sp, rcv, rp);
            m_linkIndex = m_graph->link(link);
        }
    }

    emit m_graph->graphUpdated();
}

DeleteNodes::DeleteNodes(const vector<int32_t> &selection, AbstractNodeGraph *model, const QString &name, QUndoCommand *parent) :
    m_indices(selection),
    UndoGraph(model, name, parent) {


}
void DeleteNodes::undo() {
    m_graph->loadGraph(m_document);
}
void DeleteNodes::redo() {
    QVariantList links;

    AbstractNodeGraph::NodeList list;
    for(auto &it : m_indices) {
        GraphNode *node = m_graph->node(it);
        list.push_back(node);
        QVariantList l = m_graph->saveLinks(node);
        links.append(l);
    }

    QVariantList nodes;
    for(auto it : list) {
        nodes.push_back(m_graph->saveNode(it));
        m_graph->nodeDelete(it);
    }

    QVariantMap data;
    data[gNodes] = nodes;
    data[gLinks] = links;

    m_document = data;

    emit m_graph->graphUpdated();
}

PasteNodes::PasteNodes(const QString &data, int x, int y, AbstractNodeGraph *graph, const QString &name, QUndoCommand *parent) :
    UndoGraph(graph, name, parent),
    m_document(QJsonDocument::fromJson(data.toLocal8Bit())),
    m_x(x),
    m_y(y) {

}
void PasteNodes::undo() {
    for(auto &it : m_list) {
        m_graph->nodeDelete(m_graph->node(it.toInt()));
    }
    emit m_graph->graphUpdated();
}
void PasteNodes::redo () {
    int maxX = INT_MIN;
    int maxY = INT_MIN;
    for(const QJsonValue &it : m_document.array()) {
        QVariantMap n = it.toVariant().toMap();
        maxX = qMax(maxX, n[gX].toInt());
        maxY = qMax(maxY, n[gY].toInt());
    }

    m_list.clear();
    for(const QJsonValue &it : m_document.array()) {
        QVariantMap n = it.toVariant().toMap();

        int index = -1;
        GraphNode *node = m_graph->nodeCreate(n[gType].toString(), index);
        int deltaX = maxX - n[gX].toInt();
        int deltaY = maxY - n[gY].toInt();
        node->setPosition(Vector2(m_x + deltaX, m_y + deltaY));
        m_graph->loadUserValues(node, n[gValues].toMap());

        m_list.push_back(index);
    }
    emit m_graph->graphUpdated();
}

CreateLink::CreateLink(int sender, int oport, int receiver, int iport, AbstractNodeGraph *graph, const QString &name, QUndoCommand *parent) :
        UndoGraph(graph, name, parent),
        m_sender(sender),
        m_oPort(oport),
        m_receiver(receiver),
        m_iPort(iport),
        m_index(-1) {

}
void CreateLink::undo() {
    AbstractNodeGraph::Link *link = m_graph->link(m_index);
    if(link) {
        m_graph->linkDelete(link);
        emit m_graph->graphUpdated();
    }
}
void CreateLink::redo() {
    GraphNode *snd = m_graph->node(m_sender);
    GraphNode *rcv = m_graph->node(m_receiver);
    if(snd && rcv) {
        NodePort *op = (m_oPort > -1) ? snd->port(m_oPort) : nullptr;
        NodePort *ip = (m_iPort > -1) ? rcv->port(m_iPort) : nullptr;

        AbstractNodeGraph::Link *link = m_graph->linkCreate(snd, op, rcv, ip);
        m_index = m_graph->link(link);
        emit m_graph->graphUpdated();
    }
}

DeleteLink::DeleteLink(int index, AbstractNodeGraph *graph, const QString &name, QUndoCommand *parent) :
        UndoGraph(graph, name, parent),
        m_sender(0),
        m_oPort(0),
        m_receiver(0),
        m_iPort(0),
        m_index(index) {

}
void DeleteLink::undo() {
    GraphNode *snd = m_graph->node(m_sender);
    GraphNode *rcv = m_graph->node(m_receiver);
    if(snd && rcv) {
        NodePort *op = (m_oPort > -1) ? snd->port(m_oPort) : nullptr;
        NodePort *ip = (m_iPort > -1) ? rcv->port(m_iPort) : nullptr;

        AbstractNodeGraph::Link *link = m_graph->linkCreate(snd, op, rcv, ip);
        m_index = m_graph->link(link);
        emit m_graph->graphUpdated();
    }
}
void DeleteLink::redo() {
    AbstractNodeGraph::Link *link = m_graph->link(m_index);
    if(link) {
        m_sender = m_graph->node(link->sender);
        m_receiver = m_graph->node(link->receiver);

        m_oPort = link->sender->portPosition(link->oport);
        m_iPort = link->receiver->portPosition(link->iport);

        m_graph->linkDelete(link);
        emit m_graph->graphUpdated();
    }
}

DeleteLinksByPort::DeleteLinksByPort(int node, int port, AbstractNodeGraph *graph, const QString &name, QUndoCommand *parent) :
        UndoGraph(graph, name, parent),
        m_node(node),
        m_port(port) {

}
void DeleteLinksByPort::undo() {
    for(int i = 0; i < m_links.size(); ++i) {
        Link link = *std::next(m_links.begin(), i);

        GraphNode *snd = m_graph->node(link.sender);
        GraphNode *rcv = m_graph->node(link.receiver);
        if(snd && rcv) {
            NodePort *op = (link.oport > -1) ? snd->port(link.oport) : nullptr;
            NodePort *ip = (link.iport > -1) ? rcv->port(link.iport) : nullptr;

            m_graph->linkCreate(snd, op, rcv, ip);
        }
    }
    emit m_graph->graphUpdated();
}
void DeleteLinksByPort::redo() {
    GraphNode *node = m_graph->node(m_node);
    if(node) {
        m_links.clear();
        if(m_port == -1) {
            for(auto l : m_graph->findLinks(node)) {
                m_links.push_back({
                    m_graph->node(l->sender),
                    (l->oport != nullptr) ? l->sender->portPosition(l->oport) : -1,
                    m_graph->node(l->receiver),
                    (l->iport != nullptr) ? l->receiver->portPosition(l->iport) : -1 });
            }
            m_graph->linkDelete(node);
        } else {
            NodePort *item = node->port(m_port);
            for(auto l : m_graph->findLinks(item)) {
                m_links.push_back({
                    m_graph->node(l->sender),
                    (l->oport != nullptr) ? l->sender->portPosition(l->oport) : -1,
                    m_graph->node(l->receiver),
                    (l->iport != nullptr) ? l->receiver->portPosition(l->iport) : -1 });
            }
            m_graph->linkDelete(item);
        }
        emit m_graph->graphUpdated();
    }
}
