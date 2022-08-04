#include "scheme/abstractnodegraph.h"

#include "scheme/graphnode.h"

#include <QJsonArray>
#include <QFile>
#include <QGuiApplication>
#include <QClipboard>

#define NODES       "Nodes"
#define LINKS       "Links"
#define TYPE        "Type"
#define VALUES      "Values"

#define SENDER      "Sender"
#define RECEIVER    "Receiver"
#define IPORT       "IPort"
#define OPORT       "OPort"

#define X           "X"
#define Y           "Y"

AbstractNodeGraph::AbstractNodeGraph() :
        m_rootNode(nullptr) {
}

void AbstractNodeGraph::nodeDelete(GraphNode *node) {
    if(node == m_rootNode) {
        return;
    }
    auto it = m_nodes.begin();
    while(it != m_nodes.end()) {
        if(*it == node) {
            for(NodePort *item : qAsConst(node->ports)) {
                linkDelete(item);
                delete item;
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

        Link *link     = new Link;
        link->sender   = sender;
        link->receiver = receiver;
        link->oport    = oport;
        link->iport    = iport;
        link->ptr      = nullptr;
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
    if (!loadFile.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open file.");
        return;
    }
    m_data = QJsonDocument::fromJson(loadFile.readAll()).toVariant().toMap();

    QVariantList nodes = m_data[NODES].toList();
    for(int i = 0; i < nodes.size(); ++i) {
        QVariantMap n = nodes[i].toMap();
        int32_t index = -1;
        GraphNode *node = nodeCreate(n[TYPE].toString(), index);
        node->pos = QPoint(n[X].toInt(), n[Y].toInt());
        loadUserValues(node, n[VALUES].toMap());
    }

    QVariantList links = m_data[LINKS].toList();
    for(int i = 0; i < links.size(); ++i) {
        QVariantMap l = links[i].toMap();

        GraphNode *snd = node(l[SENDER].toInt());
        GraphNode *rcv = node(l[RECEIVER].toInt());
        if(snd && rcv) {
            int index1 = l[OPORT].toInt();
            NodePort *op = (index1 > -1) ? snd->ports.at(index1) : nullptr;
            int index2 = l[IPORT].toInt();
            NodePort *ip = (index2 > -1) ? rcv->ports.at(index2) : nullptr;

            linkCreate(snd, op, rcv, ip);
        }
    }
    emit schemeUpdated();
}

QStringList AbstractNodeGraph::nodeList() const {
    return QStringList();
}

void AbstractNodeGraph::save(const QString &path) {
    QVariantList nodes;
    for(GraphNode *it : qAsConst(m_nodes)) {
        if(it != m_rootNode) {
            nodes.push_back(saveNode(it));
        }
    }
    m_data[NODES] = nodes;

    QVariantList links;
    foreach(Link *it, m_links) {
        QVariantMap link;
        link[SENDER] = node(it->sender);
        link[OPORT] = (it->oport != nullptr) ? it->sender->ports.indexOf(it->oport) : -1;
        link[RECEIVER] = node(it->receiver);
        link[IPORT] = (it->iport != nullptr) ? it->receiver->ports.indexOf(it->iport) : -1;

        links.push_back(link);
    }
    m_data[LINKS] = links;

    QFile saveFile(path);
    if(saveFile.open(QIODevice::WriteOnly)) {
        saveFile.write(QJsonDocument::fromVariant(m_data).toJson());
        saveFile.close();
    }
}

QVariant AbstractNodeGraph::saveNode(GraphNode *node) {
    QVariantMap result;
    result[TYPE] = node->type;
    result[X] = node->pos.x();
    result[Y] = node->pos.y();

    QVariantMap values;
    saveUserValues(node, values);
    result[VALUES] = values;

    return result;
}

void AbstractNodeGraph::createNode(const QString &path, int x, int y) {
    UndoManager::instance()->push(new CreateNode(path, x, y, this));
}

void AbstractNodeGraph::createAndLink(const QString &path, int x, int y, int node, int port, bool out) {
    UndoManager::instance()->push(new CreateNode(path, x, y, this, node, port, out));
}

QVariant AbstractNodeGraph::nodes() const {
    QVariantList result;

    for(GraphNode *it : m_nodes) {
        QVariantMap node;
        node["name"] = it->objectName();
        node["pos"] = it->pos;
        node["root"] = it->root;

        QVariantList ports;
        for(NodePort *p : it->ports) {
            QVariantMap port;
            port["name"] = p->m_name;
            port["out"] = p->m_out;
            port["pos"] = p->m_pos;
            port["type"] = p->m_type;

            ports.push_back(port);
        }
        node["ports"] = ports;
        result.push_back(node);

    }
    return result;
}

QVariant AbstractNodeGraph::links() const {
    QVariantList result;

    foreach(auto it, m_links) {
        QVariantMap link;
        link["sender"] = node(it->sender);
        link["receiver"] = node(it->receiver);
        link["oport"] = it->sender->ports.indexOf(it->oport);
        link["iport"] = it->receiver->ports.indexOf(it->iport);

        result.push_back(link);
    }

    return result;
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

void AbstractNodeGraph::moveNode(const QVariant selection, const QVariant nodes) {
    UndoManager::instance()->push(new MoveNode(selection, nodes, this));
}

void AbstractNodeGraph::deleteNodes(QVariant selection) {
    QVariantList list = selection.toList();
    list.removeAll(QVariant(0));
    if(!list.isEmpty()) {
        UndoManager::instance()->push(new DeleteNodes(list, this));
    }
}

void AbstractNodeGraph::copyNodes(QVariant list) {
    QJsonArray array;
    for(QVariant &it : list.toList()) {
        GraphNode *node = m_nodes.at(it.toInt());

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
        UndoScheme(graph, name, parent),
        m_node(nullptr),
        m_Path(path),
        m_Index(-1),
        m_Node(node),
        m_Port(port),
        m_Out(out),
        m_Point(x, y) {

}
void CreateNode::undo() {
    m_graph->nodeDelete(m_node);
    emit m_graph->schemeUpdated();
}
void CreateNode::redo() {
    m_node = m_graph->nodeCreate(m_Path, m_Index);
    if(m_node) {
        m_node->pos = m_Point;
    }

    if(m_Node > -1) {
        GraphNode *node = m_graph->node(m_Node);
        NodePort *item = nullptr;
        if(m_Port > -1) {
            if(node) {
                int index = 0;
                for(auto &it : node->ports) {
                    if(it->m_out == m_Out) {
                        if(index == m_Port) {
                            item = it;
                            break;
                        } else {
                            index++;
                        }
                    }
                }
            }
        }

        GraphNode *snd = (m_Out) ? node : m_node;
        GraphNode *rcv = (m_Out) ? m_node : node;
        if(snd && rcv) {
            NodePort *sp = (m_Port > -1) ? ((m_Out) ? item : snd->ports.at(0)) : nullptr;
            NodePort *rp = (m_Port > -1) ? ((m_Out) ? rcv->ports.at(0) : item) : nullptr;

            AbstractNodeGraph::Link *link = m_graph->linkCreate(snd, sp, rcv, rp);
            m_Index = m_graph->link(link);
        }
    }

    emit m_graph->schemeUpdated();
}

MoveNode::MoveNode(const QVariant &selection, const QVariant &nodes, AbstractNodeGraph *graph, const QString &name, QUndoCommand *parent) :
        UndoScheme(graph, name, parent) {

    QVariantList list = nodes.toList();
    for(auto &it : selection.toList()) {
        m_Indices.push_back(it.toInt());

        QVariantMap map = list.at(it.toInt()).toMap();
        m_Points.push_back(map["pos"].toPoint());
    }
}
void MoveNode::undo() {
    redo();
}
void MoveNode::redo() {
    QList<QPoint> positions;
    for(int i = 0; i < m_Indices.size(); i++) {
        GraphNode *node = m_graph->node(m_Indices.at(i));
        positions.push_back(node->pos);
        node->pos = m_Points.at(i);
    }
    m_Points = positions;
    emit m_graph->nodeMoved();
}

DeleteNodes::DeleteNodes(const QVariant &selection, AbstractNodeGraph *model, const QString &name, QUndoCommand *parent) :
        UndoScheme(model, name, parent) {

    for(QVariant &it : selection.toList()) {
        m_Indices.push_back(it.toInt());
    }
}
void DeleteNodes::undo() {
    QVariantMap data = m_Document.toVariant().toMap();

    QVariantList nodes = data[NODES].toList();
    for(int i = 0; i < nodes.size(); ++i) {
        QVariantMap n = nodes[i].toMap();
        int32_t index = m_Indices.at(i);
        GraphNode *node = m_graph->nodeCreate(n[TYPE].toString(), index);
        node->pos = QPoint(n[X].toInt(), n[Y].toInt());
        m_graph->loadUserValues(node, n[VALUES].toMap());
    }

    QVariantList links = data[LINKS].toList();
    for(int i = 0; i < links.size(); ++i) {
        QVariantMap l = links[i].toMap();

        GraphNode *snd = m_graph->node(l[SENDER].toInt());
        GraphNode *rcv = m_graph->node(l[RECEIVER].toInt());
        if(snd && rcv) {
            int index1 = l[OPORT].toInt();
            NodePort *op = (index1 > -1) ? snd->ports.at(index1) : nullptr;
            int index2 = l[IPORT].toInt();
            NodePort *ip = (index2 > -1) ? rcv->ports.at(index2) : nullptr;

            m_graph->linkCreate(snd, op, rcv, ip);
        }
    }

    emit m_graph->schemeUpdated();
}
void DeleteNodes::redo() {
    QVariantList links;

    AbstractNodeGraph::NodeList list;
    for(auto &it : m_Indices) {
        GraphNode *node = m_graph->node(it);
        list.push_back(node);
        for(auto l : m_graph->findLinks(node)) {
            QVariantMap link;
            link[SENDER] = m_graph->node(l->sender);
            link[OPORT] = (l->oport != nullptr) ? l->sender->ports.indexOf(l->oport) : -1;
            link[RECEIVER] = m_graph->node(l->receiver);
            link[IPORT] = (l->iport != nullptr) ? l->receiver->ports.indexOf(l->iport) : -1;

            links.push_back(link);
        }
    }

    QVariantList nodes;
    for(auto it : list) {
        nodes.push_back(m_graph->saveNode(it));
        m_graph->nodeDelete(it);
    }

    QVariantMap data;
    data[NODES] = nodes;
    data[LINKS] = links;

    m_Document = QJsonDocument::fromVariant(data);

    emit m_graph->schemeUpdated();
}

PasteNodes::PasteNodes(const QString &data, int x, int y, AbstractNodeGraph *graph, const QString &name, QUndoCommand *parent) :
        UndoScheme(graph, name, parent),
        m_Document(QJsonDocument::fromJson(data.toLocal8Bit())),
        m_X(x),
        m_Y(y) {

}
void PasteNodes::undo() {
    for(auto &it : m_List) {
        m_graph->nodeDelete(m_graph->node(it.toInt()));
    }
    emit m_graph->schemeUpdated();
}
void PasteNodes::redo () {
    int maxX = INT_MIN;
    int maxY = INT_MIN;
    for(QJsonValue it : m_Document.array()) {
        QVariantMap n = it.toVariant().toMap();
        maxX = qMax(maxX, n[X].toInt());
        maxY = qMax(maxY, n[Y].toInt());
    }

    m_List.clear();
    for(QJsonValue it : m_Document.array()) {
        QVariantMap n = it.toVariant().toMap();

        int index = -1;
        GraphNode *node = m_graph->nodeCreate(n[TYPE].toString(), index);
        int deltaX = maxX - n[X].toInt();
        int deltaY = maxY - n[Y].toInt();
        node->pos = QPoint(m_X + deltaX, m_Y + deltaY);
        m_graph->loadUserValues(node, n[VALUES].toMap());

        m_List.push_back(index);
    }
    emit m_graph->schemeUpdated();
}

CreateLink::CreateLink(int sender, int oport, int receiver, int iport, AbstractNodeGraph *graph, const QString &name, QUndoCommand *parent) :
        UndoScheme(graph, name, parent),
        m_Sender(sender),
        m_OPort(oport),
        m_Receiver(receiver),
        m_IPort(iport),
        m_Index(-1) {

}
void CreateLink::undo() {
    AbstractNodeGraph::Link *link = m_graph->link(m_Index);
    if(link) {
        m_graph->linkDelete(link);
        emit m_graph->schemeUpdated();
    }
}
void CreateLink::redo() {
    GraphNode *snd = m_graph->node(m_Sender);
    GraphNode *rcv = m_graph->node(m_Receiver);
    if(snd && rcv) {
        NodePort *op = (m_OPort > -1) ? snd->ports.at(m_OPort) : nullptr;
        NodePort *ip = (m_IPort > -1) ? rcv->ports.at(m_IPort) : nullptr;

        AbstractNodeGraph::Link *link = m_graph->linkCreate(snd, op, rcv, ip);
        m_Index = m_graph->link(link);
        emit m_graph->schemeUpdated();
    }
}

DeleteLink::DeleteLink(int index, AbstractNodeGraph *graph, const QString &name, QUndoCommand *parent) :
        UndoScheme(graph, name, parent),
        m_Sender(0),
        m_OPort(0),
        m_Receiver(0),
        m_IPort(0),
        m_Index(index) {

}
void DeleteLink::undo() {
    GraphNode *snd = m_graph->node(m_Sender);
    GraphNode *rcv = m_graph->node(m_Receiver);
    if(snd && rcv) {
        NodePort *op = (m_OPort > -1) ? snd->ports.at(m_OPort) : nullptr;
        NodePort *ip = (m_IPort > -1) ? rcv->ports.at(m_IPort) : nullptr;

        AbstractNodeGraph::Link *link = m_graph->linkCreate(snd, op, rcv, ip);
        m_Index = m_graph->link(link);
        emit m_graph->schemeUpdated();
    }
}
void DeleteLink::redo() {
    AbstractNodeGraph::Link *link = m_graph->link(m_Index);
    if(link) {
        m_Sender = m_graph->node(link->sender);
        m_Receiver = m_graph->node(link->receiver);

        m_OPort = link->sender->ports.indexOf(link->oport);
        m_IPort = link->receiver->ports.indexOf(link->iport);

        m_graph->linkDelete(link);
        emit m_graph->schemeUpdated();
    }
}

DeleteLinksByPort::DeleteLinksByPort(int node, int port, AbstractNodeGraph *graph, const QString &name, QUndoCommand *parent) :
        UndoScheme(graph, name, parent),
        m_node(node),
        m_port(port) {

}
void DeleteLinksByPort::undo() {
    for(int i = 0; i < m_links.size(); ++i) {
        QVariantMap l = m_links[i].toMap();

        GraphNode *snd = m_graph->node(l[SENDER].toInt());
        GraphNode *rcv = m_graph->node(l[RECEIVER].toInt());
        if(snd && rcv) {
            int index1 = l[OPORT].toInt();
            NodePort *op = (index1 > -1) ? snd->ports.at(index1) : nullptr;
            int index2 = l[IPORT].toInt();
            NodePort *ip = (index2 > -1) ? rcv->ports.at(index2) : nullptr;

            m_graph->linkCreate(snd, op, rcv, ip);
        }
    }
    emit m_graph->schemeUpdated();
}
void DeleteLinksByPort::redo() {
    GraphNode *node = m_graph->node(m_node);
    if(node) {
        m_links.clear();
        if(m_port == -1) {
            for(auto l : m_graph->findLinks(node)) {
                QVariantMap link;
                link[SENDER] = m_graph->node(l->sender);
                link[OPORT] = (l->oport != nullptr) ? l->sender->ports.indexOf(l->oport) : -1;
                link[RECEIVER] = m_graph->node(l->receiver);
                link[IPORT] = (l->iport != nullptr) ? l->receiver->ports.indexOf(l->iport) : -1;

                m_links.push_back(link);
            }
            m_graph->linkDelete(node);
        } else {
            NodePort *item = node->ports.at(m_port);
            for(auto l : m_graph->findLinks(item)) {
                QVariantMap link;
                link[SENDER] = m_graph->node(l->sender);
                link[OPORT] = (l->oport != nullptr) ? l->sender->ports.indexOf(l->oport) : -1;
                link[RECEIVER] = m_graph->node(l->receiver);
                link[IPORT] = (l->iport != nullptr) ? l->receiver->ports.indexOf(l->iport) : -1;

                m_links.push_back(link);
            }
            m_graph->linkDelete(item);
        }
        emit m_graph->schemeUpdated();
    }
}
