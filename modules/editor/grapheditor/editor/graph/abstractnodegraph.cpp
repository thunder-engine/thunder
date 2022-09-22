#include "abstractnodegraph.h"

#include "graphnode.h"

#include <QJsonArray>
#include <QFile>
#include <QGuiApplication>
#include <QClipboard>

#define NODES    "Nodes"
#define LINKS    "Links"
#define TYPE     "Type"
#define VALUES   "Values"
#define NAME     "Name"

#define SENDER   "Sender"
#define RECEIVER "Receiver"
#define IPORT    "IPort"
#define OPORT    "OPort"

#define X        "X"
#define Y        "Y"

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
            for(NodePort *item : qAsConst(node->m_ports)) {
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
        node->m_pos = QPoint(n[X].toInt(), n[Y].toInt());
        loadUserValues(node, n[VALUES].toMap());
    }

    QVariantList links = m_data[LINKS].toList();
    for(int i = 0; i < links.size(); ++i) {
        QVariantMap l = links[i].toMap();

        GraphNode *snd = node(l[SENDER].toInt());
        GraphNode *rcv = node(l[RECEIVER].toInt());
        if(snd && rcv) {
            int index1 = l[OPORT].toInt();
            NodePort *op = (index1 > -1) ? snd->m_ports.at(index1) : nullptr;
            int index2 = l[IPORT].toInt();
            NodePort *ip = (index2 > -1) ? rcv->m_ports.at(index2) : nullptr;

            linkCreate(snd, op, rcv, ip);
        }
    }
    emit graphUpdated();
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
        link[OPORT] = (it->oport != nullptr) ? it->sender->m_ports.indexOf(it->oport) : -1;
        link[RECEIVER] = node(it->receiver);
        link[IPORT] = (it->iport != nullptr) ? it->receiver->m_ports.indexOf(it->iport) : -1;

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
    result[TYPE] = node->m_type;
    result[X] = node->m_pos.x();
    result[Y] = node->m_pos.y();

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

const AbstractNodeGraph::NodeList &AbstractNodeGraph::nodes() const {
    return m_nodes;
}

QVariant AbstractNodeGraph::links() const {
    QVariantList result;

    foreach(auto it, m_links) {
        QVariantMap link;
        link["sender"] = node(it->sender);
        link["receiver"] = node(it->receiver);
        link["oport"] = it->sender->m_ports.indexOf(it->oport);
        link["iport"] = it->receiver->m_ports.indexOf(it->iport);

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
        m_node->m_pos = m_point;
    }

    if(m_fromNode > -1) {
        GraphNode *node = m_graph->node(m_fromNode);
        NodePort *item = nullptr;
        if(m_fromPort > -1) {
            if(node) {
                int index = 0;
                for(auto &it : node->m_ports) {
                    if(it->m_out == m_out) {
                        if(index == m_fromPort) {
                            item = it;
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
            NodePort *sp = (m_fromPort > -1) ? ((m_out) ? item : snd->m_ports.at(0)) : nullptr;
            NodePort *rp = (m_fromPort > -1) ? ((m_out) ? rcv->m_ports.at(0) : item) : nullptr;

            AbstractNodeGraph::Link *link = m_graph->linkCreate(snd, sp, rcv, rp);
            m_linkIndex = m_graph->link(link);
        }
    }

    emit m_graph->graphUpdated();
}

MoveNode::MoveNode(const QVariant &selection, const QVariant &nodes, AbstractNodeGraph *graph, const QString &name, QUndoCommand *parent) :
        UndoScheme(graph, name, parent) {

    QVariantList list = nodes.toList();
    for(auto &it : selection.toList()) {
        m_indices.push_back(it.toInt());

        QVariantMap map = list.at(it.toInt()).toMap();
        m_points.push_back(map["pos"].toPoint());
    }
}
void MoveNode::undo() {
    redo();
}
void MoveNode::redo() {
    QList<QPoint> positions;
    for(int i = 0; i < m_indices.size(); i++) {
        GraphNode *node = m_graph->node(m_indices.at(i));
        positions.push_back(node->m_pos);
        node->m_pos = m_points.at(i);
    }
    m_points = positions;
    emit m_graph->nodeMoved();
}

DeleteNodes::DeleteNodes(const QVariant &selection, AbstractNodeGraph *model, const QString &name, QUndoCommand *parent) :
        UndoScheme(model, name, parent) {

    for(QVariant &it : selection.toList()) {
        m_indices.push_back(it.toInt());
    }
}
void DeleteNodes::undo() {
    QVariantMap data = m_document.toVariant().toMap();

    QVariantList nodes = data[NODES].toList();
    for(int i = 0; i < nodes.size(); ++i) {
        QVariantMap n = nodes[i].toMap();
        int32_t index = m_indices.at(i);
        GraphNode *node = m_graph->nodeCreate(n[TYPE].toString(), index);
        node->m_pos = QPoint(n[X].toInt(), n[Y].toInt());
        m_graph->loadUserValues(node, n[VALUES].toMap());
    }

    QVariantList links = data[LINKS].toList();
    for(int i = 0; i < links.size(); ++i) {
        QVariantMap l = links[i].toMap();

        GraphNode *snd = m_graph->node(l[SENDER].toInt());
        GraphNode *rcv = m_graph->node(l[RECEIVER].toInt());
        if(snd && rcv) {
            int index1 = l[OPORT].toInt();
            NodePort *op = (index1 > -1) ? snd->m_ports.at(index1) : nullptr;
            int index2 = l[IPORT].toInt();
            NodePort *ip = (index2 > -1) ? rcv->m_ports.at(index2) : nullptr;

            m_graph->linkCreate(snd, op, rcv, ip);
        }
    }

    emit m_graph->graphUpdated();
}
void DeleteNodes::redo() {
    QVariantList links;

    AbstractNodeGraph::NodeList list;
    for(auto &it : m_indices) {
        GraphNode *node = m_graph->node(it);
        list.push_back(node);
        for(auto l : m_graph->findLinks(node)) {
            QVariantMap link;
            link[SENDER] = m_graph->node(l->sender);
            link[OPORT] = (l->oport != nullptr) ? l->sender->m_ports.indexOf(l->oport) : -1;
            link[RECEIVER] = m_graph->node(l->receiver);
            link[IPORT] = (l->iport != nullptr) ? l->receiver->m_ports.indexOf(l->iport) : -1;

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

    m_document = QJsonDocument::fromVariant(data);

    emit m_graph->graphUpdated();
}

PasteNodes::PasteNodes(const QString &data, int x, int y, AbstractNodeGraph *graph, const QString &name, QUndoCommand *parent) :
        UndoScheme(graph, name, parent),
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
        maxX = qMax(maxX, n[X].toInt());
        maxY = qMax(maxY, n[Y].toInt());
    }

    m_list.clear();
    for(const QJsonValue &it : m_document.array()) {
        QVariantMap n = it.toVariant().toMap();

        int index = -1;
        GraphNode *node = m_graph->nodeCreate(n[TYPE].toString(), index);
        int deltaX = maxX - n[X].toInt();
        int deltaY = maxY - n[Y].toInt();
        node->m_pos = QPoint(m_x + deltaX, m_y + deltaY);
        m_graph->loadUserValues(node, n[VALUES].toMap());

        m_list.push_back(index);
    }
    emit m_graph->graphUpdated();
}

CreateLink::CreateLink(int sender, int oport, int receiver, int iport, AbstractNodeGraph *graph, const QString &name, QUndoCommand *parent) :
        UndoScheme(graph, name, parent),
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
        NodePort *op = (m_oPort > -1) ? snd->m_ports.at(m_oPort) : nullptr;
        NodePort *ip = (m_iPort > -1) ? rcv->m_ports.at(m_iPort) : nullptr;

        AbstractNodeGraph::Link *link = m_graph->linkCreate(snd, op, rcv, ip);
        m_index = m_graph->link(link);
        emit m_graph->graphUpdated();
    }
}

DeleteLink::DeleteLink(int index, AbstractNodeGraph *graph, const QString &name, QUndoCommand *parent) :
        UndoScheme(graph, name, parent),
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
        NodePort *op = (m_oPort > -1) ? snd->m_ports.at(m_oPort) : nullptr;
        NodePort *ip = (m_iPort > -1) ? rcv->m_ports.at(m_iPort) : nullptr;

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

        m_oPort = link->sender->m_ports.indexOf(link->oport);
        m_iPort = link->receiver->m_ports.indexOf(link->iport);

        m_graph->linkDelete(link);
        emit m_graph->graphUpdated();
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
            NodePort *op = (index1 > -1) ? snd->m_ports.at(index1) : nullptr;
            int index2 = l[IPORT].toInt();
            NodePort *ip = (index2 > -1) ? rcv->m_ports.at(index2) : nullptr;

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
                QVariantMap link;
                link[SENDER] = m_graph->node(l->sender);
                link[OPORT] = (l->oport != nullptr) ? l->sender->m_ports.indexOf(l->oport) : -1;
                link[RECEIVER] = m_graph->node(l->receiver);
                link[IPORT] = (l->iport != nullptr) ? l->receiver->m_ports.indexOf(l->iport) : -1;

                m_links.push_back(link);
            }
            m_graph->linkDelete(node);
        } else {
            NodePort *item = node->m_ports.at(m_port);
            for(auto l : m_graph->findLinks(item)) {
                QVariantMap link;
                link[SENDER] = m_graph->node(l->sender);
                link[OPORT] = (l->oport != nullptr) ? l->sender->m_ports.indexOf(l->oport) : -1;
                link[RECEIVER] = m_graph->node(l->receiver);
                link[IPORT] = (l->iport != nullptr) ? l->receiver->m_ports.indexOf(l->iport) : -1;

                m_links.push_back(link);
            }
            m_graph->linkDelete(item);
        }
        emit m_graph->graphUpdated();
    }
}
