#include "abstractnodegraph.h"

#include "graphnode.h"

#include <QJsonArray>
#include <QFile>
#include <QGuiApplication>
#include <QClipboard>
#include <QFileInfo>

namespace {
    // Old Format
    const char *gOldNodes("Nodes");
    const char *gOldLinks("Links");
    const char *gOldValues("Values");

    const char *gOldType("Type");
    const char *gOldIndex("Index");

    const char *gOldSender("Sender");
    const char *gOldReceiver("Receiver");
    const char *gIPort("IPort");
    const char *gOPort("OPort");

    const char *gOldX("X");
    const char *gOldY("Y");

    // New Format
    const char *gGraph("graph");
    const char *gNodes("nodes");
    const char *gNode("node");
    const char *gLinks("links");
    const char *gLink("link");
    const char *gValues("values");

    const char *gType("type");
    const char *gIndex("index");
    const char *gName("name");

    const char *gSender("sender");
    const char *gReceiver("receiver");
    const char *gIn("in");
    const char *gOut("out");

    const char *gX("x");
    const char *gY("y");
}

AbstractNodeGraph::AbstractNodeGraph() :
        m_rootNode(nullptr),
        m_version(0) {
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

        if((oport && iport && oport->m_call == iport->m_call) ||
           (oport == nullptr && iport == nullptr)) {
            Link *link = new Link;
            link->sender = sender;
            link->receiver = receiver;
            link->oport = oport;
            link->iport = iport;
            link->ptr = nullptr;
            m_links.push_back(link);

            emit sender->updated();
            emit receiver->updated();

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

            emit first->updated();
            emit second->updated();
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
            emit second->updated();
        } else {
            ++it;
        }
    }
    emit node->updated();
}

void AbstractNodeGraph::linkDelete(Link *link) {
    auto it = m_links.begin();
    while(it != m_links.end()) {
        if(*it == link) {
            GraphNode *first = link->sender;
            GraphNode *second = link->receiver;

            m_links.erase(it);
            delete link;

            emit first->updated();
            emit second->updated();

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
    return (index > -1 && index < m_nodes.size()) ? m_nodes.at(index) : nullptr;
}

AbstractNodeGraph::Link *AbstractNodeGraph::link(int index) {
    return (index > -1 && index < m_links.size()) ? m_links.at(index) : nullptr;
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
    m_rootNode->setObjectName(QFileInfo(path).baseName());
    m_nodes.push_back(m_rootNode);

    QFile loadFile(path);
    if(!loadFile.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open file.");
        return;
    }
    QByteArray data(loadFile.readAll());
    loadFile.close();

    if(data.at(0) == '{') { // Load old json format
        loadGraphV0(QJsonDocument::fromJson(data).toVariant().toMap());
        save(path); // Need to save in new xml format
    } else if(data.at(0) == '<') { // Load new xml format
        QDomDocument doc;
        doc.setContent(data);

        QDomElement document = doc.documentElement();
        int version = document.attribute("version", "0").toInt();

        if(version == 0) {
            loadGraphV0(loadXmlMap(document));
        } else {
            QDomNode p = document.firstChild();
            while(!p.isNull()) {
                QDomElement element = p.toElement();
                if(!element.isNull()) {
                    loadGraphV11(element);
                }

                p = p.nextSiblingElement();
            }

            emit graphUpdated();
        }

        if(version != m_version) {
            save(path);
        }
    }
}

void AbstractNodeGraph::save(const QString &path) {
    QDomDocument xml;

    QDomElement document = xml.createElement("document");

    document.setAttribute("version", m_version);

    saveGraph(document, xml);

    xml.appendChild(document);

    QFile saveFile(path);
    if(saveFile.open(QIODevice::WriteOnly)) {
        saveFile.write(xml.toByteArray(4));
        saveFile.close();
    }
}

QStringList AbstractNodeGraph::nodeList() const {
    return QStringList();
}

void AbstractNodeGraph::setPreviewVisible(GraphNode *node, bool visible) {

}

Texture *AbstractNodeGraph::preview(GraphNode *node) {
    return nullptr;
}

QVariantMap AbstractNodeGraph::loadXmlMap(const QDomElement &parent) {
    QVariantMap result;

    QDomNode n = parent.firstChild();
    while(!n.isNull()) {
        QDomElement e = n.toElement();
        if(!e.isNull()) {
            QString key = e.attribute("Name");

            switch(e.attribute(gOldType).toInt()) {
                case QMetaType::Bool: result[key] = (e.text() == "true"); break;
                case QMetaType::Int: result[key] = e.text().toInt(); break;
                case QMetaType::Float:
                case QMetaType::Double: result[key] = e.text().toFloat(); break;
                case QMetaType::QString: result[key] = e.text(); break;
                case QVariant::Map: result[key] = loadXmlMap(e); break;
                case QVariant::List: result[key] = loadXmlList(e); break;
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
            switch(e.attribute(gOldType).toInt()) {
                case QMetaType::Bool: result.push_back(e.text() == "true"); break;
                case QMetaType::Int: result.push_back(e.text().toInt()); break;
                case QMetaType::UInt: result.push_back(e.text().toUInt()); break;
                case QMetaType::Float:
                case QMetaType::Double: result.push_back(e.text().toFloat()); break;
                case QMetaType::QString: result.push_back(e.text()); break;
                case QMetaType::QVariantMap: result.push_back(loadXmlMap(e)); break;
                case QMetaType::QVariantList: result.push_back(loadXmlList(e)); break;
                default: break;
            }
        }
        n = n.nextSibling();
    }

    return result;
}

void AbstractNodeGraph::loadGraphV0(const QVariantMap &data) {
    QVariantList nodes = data[gOldNodes].toList();
    for(int i = 0; i < nodes.size(); ++i) {
        QVariantMap n = nodes[i].toMap();
        int32_t index = n[gOldIndex].isValid() ? n[gOldIndex].toInt() : -1;
        QString type = n[gOldType].toString();
        GraphNode *node = nodeCreate(type, index);
        if(node) {
            node->setPosition(Vector2(n[gOldX].toInt(), n[gOldY].toInt()));
            loadUserValues(node, n[gOldValues].toMap());
        }
    }

    QVariantList links = data[gOldLinks].toList();
    for(int i = 0; i < links.size(); ++i) {
        QVariantMap l = links[i].toMap();

        GraphNode *snd = node(l[gOldSender].toInt());
        GraphNode *rcv = node(l[gOldReceiver].toInt());

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

void AbstractNodeGraph::loadGraphV11(const QDomElement &parent) {
    if(parent.tagName() == gGraph) {
        QDomElement nodes = parent.firstChildElement(gNodes);
        if(!nodes.isNull()) {
            QDomElement nodeElement = nodes.firstChildElement();
            while(!nodeElement.isNull()) {
                int32_t index = nodeElement.attribute(gIndex, "-1").toInt();
                QString type = nodeElement.attribute(gType);
                GraphNode *node = nodeCreate(type, index);
                if(node) {
                    node->setPosition(Vector2(nodeElement.attribute(gX).toInt(),
                                              nodeElement.attribute(gY).toInt()));

                    QVariantMap values;
                    QDomElement valueElement = nodeElement.firstChildElement("value");
                    while(!valueElement.isNull()) {
                        QString type = valueElement.attribute(gType);
                        QString name = valueElement.attribute(gName);
                        if(type == "bool") {
                            values[name] = (valueElement.text() == "true");
                        } else if(type == "int") {
                            values[name] = valueElement.text().toInt();
                        } else if(type == "float") {
                            values[name] = valueElement.text().toFloat();
                        } else if(type == "string") {
                            values[name] = valueElement.text();
                        } else if(type == "Vector2" || type == "Vector3" || type == "Vector4") {
                            QVariantList list;
                            list.push_back(type);
                            for(auto &it : valueElement.text().split(", ")) {
                                list.push_back(it.toFloat());
                            }
                            values[name] = list;
                        } else if(type == "Template") {
                            QVariantList list;
                            list.push_back(type);
                            for(auto &it : valueElement.text().split(", ")) {
                                list.push_back(it);
                            }
                            values[name] = list;
                        } else if(type == "Color") {
                            QVariantList list;
                            list.push_back(type);
                            for(auto &it : valueElement.text().split(", ")) {
                                list.push_back(it);
                            }
                            values[name] = list;
                        }

                        valueElement = valueElement.nextSiblingElement();
                    }

                    loadUserValues(node, values);
                }

                nodeElement = nodeElement.nextSiblingElement();
            }
        }

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

void AbstractNodeGraph::saveGraph(QDomElement parent, QDomDocument xml) const {
    QDomElement graph = xml.createElement(gGraph);

    QVariantList nodes;
    QVariantList links;

    for(GraphNode *it : qAsConst(m_nodes)) {
        if(it != m_rootNode) {
            nodes.push_back(saveNode(it));
        }

        links.append(saveLinks(it));
    }

    QDomElement nodesElement = xml.createElement(gNodes);
    for(auto &node : nodes) {
        QDomElement nodeElement = xml.createElement(gNode);

        QVariantMap fields = node.toMap();
        for(auto &key : fields.keys()) {
            if(key.toLower() == gValues) {
                QVariantMap values = fields.value(key).toMap();

                for(auto &value : values.keys()) {
                    QDomElement valueElement = xml.createElement("value");
                    valueElement.setAttribute(gName, value);

                    QVariant v = values.value(value);
                    switch(v.type()) {
                        case QVariant::List: {
                            QVariantList list = v.toList();
                            QString type = list.front().toString();
                            valueElement.setAttribute(gType, type);
                            list.pop_front();
                            QString pack;
                            for(auto &it : list) {
                                pack += it.toString() + ", ";
                            }
                            pack.resize(pack.size() - 2);
                            valueElement.appendChild(xml.createTextNode(pack));
                        } break;
                        default: {
                            QString type = v.typeName();
                            if(type == "QString") {
                                type = "string";
                            }
                            valueElement.setAttribute(gType, type);
                            valueElement.appendChild(xml.createTextNode(v.toString()));
                        } break;
                    }

                    nodeElement.appendChild(valueElement);
                }
            } else {
                nodeElement.setAttribute(key, fields.value(key).toString());
            }
        }
        nodesElement.appendChild(nodeElement);
    }

    QDomElement linksElement = xml.createElement(gLinks);
    for(auto &link : links) {
        QDomElement linkElement = xml.createElement(gLink);

        QVariantMap fields = link.toMap();
        for(auto &key : fields.keys()) {
            linkElement.setAttribute(key, fields.value(key).toString());
        }

        linksElement.appendChild(linkElement);
    }

    graph.appendChild(nodesElement);
    graph.appendChild(linksElement);

    parent.appendChild(graph);
}

QVariantMap AbstractNodeGraph::saveNode(GraphNode *node) const {
    QVariantMap result;
    result[gType] = node->typeName().c_str();
    result[gX] = (int)node->position().x;
    result[gY] = (int)node->position().y;
    result[gIndex] = AbstractNodeGraph::node(node);

    QVariantMap values;
    saveUserValues(node, values);
    result[gValues] = values;

    return result;
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

void AbstractNodeGraph::deleteNodes(const std::vector<int32_t> &selection) {
    UndoManager::instance()->push(new DeleteNodes(selection, this));
}

void AbstractNodeGraph::copyNodes(const std::vector<int32_t> &selection) {
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

DeleteNodes::DeleteNodes(const std::vector<int32_t> &selection, AbstractNodeGraph *model, const QString &name, QUndoCommand *parent) :
    m_indices(selection),
    UndoGraph(model, name, parent) {

}
void DeleteNodes::undo() {
    m_graph->loadGraphV0(m_document);
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
