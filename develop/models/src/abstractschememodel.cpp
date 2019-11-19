#include "abstractschememodel.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>
#include <QGuiApplication>
#include <QClipboard>
#include <QDebug>

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

AbstractSchemeModel::AbstractSchemeModel() :
        m_pRootNode(nullptr) {

    m_pRootNode = new Node;
    m_pRootNode->root = true;
    m_pRootNode->name = "";
    m_pRootNode->ptr = this;

    m_Nodes.push_back(m_pRootNode);
}

void AbstractSchemeModel::nodeDelete(Node *node) {
    if(node == m_pRootNode) {
        return;
    }
    auto it = m_Nodes.begin();
    while(it != m_Nodes.end()) {
        if(*it == node) {
            for(Item *item : node->list) {
                linkDelete(item, true);
                delete item;
            }

            for(auto link : m_Links) {
                if(link->sender == node || link->receiver == node) {
                    m_Links.removeAll(link);
                    delete link;
                }
            }

            it  = m_Nodes.erase(it);
            delete node;
            break;
        } else {
            ++it;
        }
    }
}

void AbstractSchemeModel::linkCreate(Node *sender, Item *oport, Node *receiver, Item *iport) {
    bool result = true;
    for(auto it : m_Links) {
        if(it->sender == sender && it->receiver == receiver &&
           it->oport == oport && it->iport == iport) {
            result  = false;
            break;
        }
    }
    if(result) {
        for(auto it : m_Links) {
            if(it->iport == iport && iport != nullptr) {
                linkDelete(iport);
            }
        }

        Link *link      = new Link;
        link->sender    = sender;
        link->receiver  = receiver;
        link->oport     = oport;
        link->iport     = iport;
        link->ptr       = nullptr;
        m_Links.push_back(link);

        emit schemeUpdated();
    }
}

void AbstractSchemeModel::linkDelete(Item *item, bool silent) {
    bool result = false;
    auto it = m_Links.begin();
    while(it != m_Links.end()) {
        Link *link = *it;
        if(link->oport == item || link->iport == item) {
            it  = m_Links.erase(it);
            delete link;
            result  = true;
        } else {
            ++it;
        }
    }
    if(result && !silent) {
        emit schemeUpdated();
    }
}

const AbstractSchemeModel::LinkList AbstractSchemeModel::findLinks(const Node *node) const {
    LinkList result;
    for(const auto it : m_Links) {
        if(it->receiver == node || it->sender == node) {
            result.push_back(it);
        }
    }
    return result;
}

const AbstractSchemeModel::Link *AbstractSchemeModel::findLink(const Node *node, const char *item) const {
    for(const auto it : m_Links) {
        if(it->receiver == node && it->iport->name.compare(item) == 0) {
            return it;
        }
    }
    return nullptr;
}

AbstractSchemeModel::Node *AbstractSchemeModel::node(int index) {
    return m_Nodes.at(index);
}

AbstractSchemeModel::Link *AbstractSchemeModel::link(int index) {
    return m_Links.at(index);
}

void AbstractSchemeModel::load(const QString &path) {
    foreach(Link *it, m_Links) {
        delete it;
    }
    m_Links.clear();

    foreach(Node *it, m_Nodes) {
        if(it != m_pRootNode) {
            delete it;
        }
    }
    m_Nodes.clear();
    m_Nodes.push_back(m_pRootNode);

    QFile loadFile(path);
    if (!loadFile.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open file.");
        return;
    }
    m_Data = QJsonDocument::fromJson(loadFile.readAll()).toVariant().toMap();

    QVariantList nodes = m_Data[NODES].toList();
    for(int i = 0; i < nodes.size(); ++i) {
        QVariantMap n = nodes[i].toMap();
        int32_t index = -1;
        Node *node = nodeCreate(n[TYPE].toString(), index);
        node->pos = QPoint(n[X].toInt(), n[Y].toInt());
        loadUserValues(node, n[VALUES].toMap());
    }

    QVariantList links = m_Data[LINKS].toList();
    for(int i = 0; i < links.size(); ++i) {
        QVariantMap l = links[i].toMap();
        createLink(l[SENDER].toInt(), l[OPORT].toInt(), l[RECEIVER].toInt(), l[IPORT].toInt());
    }
}

void AbstractSchemeModel::save(const QString &path) {
    QVariantList nodes;
    for(Node *it : m_Nodes) {
        if(it != m_pRootNode) {
            nodes.push_back(saveNode(it));
        }
    }
    m_Data[NODES] = nodes;

    QVariantList links;
    for(Link *it : m_Links) {
        QVariantMap link;
        link[SENDER] = m_Nodes.indexOf(it->sender);
        link[OPORT] = (it->oport != nullptr) ? it->sender->list.indexOf(it->oport) : -1;
        link[RECEIVER] = m_Nodes.indexOf(it->receiver);
        link[IPORT] = (it->iport != nullptr) ? it->receiver->list.indexOf(it->iport) : -1;

        links.push_back(link);
    }
    m_Data[LINKS] = links;

    QFile saveFile(path);
    if(saveFile.open(QIODevice::WriteOnly)) {
        saveFile.write(QJsonDocument::fromVariant(m_Data).toJson());
        saveFile.close();
    }
}

QVariant AbstractSchemeModel::saveNode(Node *node) {
    QVariantMap result;
    result[TYPE] = node->type;
    result[X] = node->pos.x();
    result[Y] = node->pos.y();

    QVariantMap values;
    saveUserValues(node, values);
    result[VALUES] = values;

    return result;
}

void AbstractSchemeModel::createNode(const QString &path, int x, int y) {
    UndoManager::instance()->push(new CreateNode(path, x, y, this));
}

QVariant AbstractSchemeModel::nodes() const {
    QVariantList result;

    foreach(auto it, m_Nodes) {
        QVariantMap node;
        node["name"] = it->name;
        node["pos"] = it->pos;
        node["root"] = it->root;

        QVariantList ports;
        foreach(auto p, it->list) {
            QVariantMap port;
            port["name"] = p->name;
            port["out"] = p->out;
            port["pos"] = p->pos;

            ports.push_back(port);
        }
        node["ports"] = ports;
        result.push_back(node);
    }
    return result;
}

QVariant AbstractSchemeModel::links() const {
    QVariantList result;

    foreach(auto it, m_Links) {
        QVariantMap link;
        link["sender"] = m_Nodes.indexOf(it->sender);
        link["receiver"] = m_Nodes.indexOf(it->receiver);
        link["oport"] = it->sender->list.indexOf(it->oport);
        link["iport"] = it->receiver->list.indexOf(it->iport);

        result.push_back(link);
    }

    return result;
}

void AbstractSchemeModel::createLink(int sender, int oport, int receiver, int iport) {
    Node *snd = m_Nodes.at(sender);
    Node *rcv = m_Nodes.at(receiver);
    if(snd && rcv) {
        Item *op = (oport > -1) ? snd->list.at(oport) : nullptr;
        Item *ip = (iport > -1) ? rcv->list.at(iport) : nullptr;

        linkCreate(snd, op, rcv, ip);
    }
}

void AbstractSchemeModel::deleteLink(int index) {
    if(index > -1) {
        Link *link = m_Links.at(index);
        delete link;
        m_Links.removeAt(index);

        emit schemeUpdated();
    }
}

void AbstractSchemeModel::deleteLinksByNode(int index, int port) {
    Node *node = m_Nodes.at(index);
    if(node) {
        if(port == -1) {
            auto it = m_Links.begin();
            while(it != m_Links.end()) {
                Link *link = *it;
                if(link->sender == node || link->receiver == node) {
                    it  = m_Links.erase(it);
                    delete link;
                } else {
                    ++it;
                }
            }
            emit schemeUpdated();
        } else {
            linkDelete(node->list.at(port));
        }
    }
}

void AbstractSchemeModel::moveNode(const QVariant selection, const QVariant nodes) {
    UndoManager::instance()->push(new MoveNode(selection, nodes, this));
}

void AbstractSchemeModel::deleteNodes(QVariant list) {
    UndoManager::instance()->push(new DeleteNodes(list, this));
}

void AbstractSchemeModel::copyNodes(QVariant list) {
    QJsonArray array;
    for(QVariant it : list.toList()) {
        Node *node = m_Nodes.at(it.toInt());

        array.push_back(QJsonValue::fromVariant(saveNode(node)));
    }

    QGuiApplication::clipboard()->setText(QJsonDocument(array).toJson());
}

QVariant AbstractSchemeModel::pasteNodes(int x, int y) {
    PasteNodes *paste = new PasteNodes(QGuiApplication::clipboard()->text(), x, y, this);
    UndoManager::instance()->push(paste);
    return paste->list();
}

CreateNode::CreateNode(const QString &path, int x, int y, AbstractSchemeModel *model, const QString &name, QUndoCommand *parent) :
        UndoScheme(model, name, parent),
        m_pNode(nullptr),
        m_Path(path),
        m_Index(-1),
        m_Point(x, y) {

}
void CreateNode::undo() {
    m_pModel->nodeDelete(m_pNode);
    m_pModel->schemeUpdated();
}
void CreateNode::redo() {
    m_pNode = m_pModel->nodeCreate(m_Path, m_Index);
    if(m_pNode) {
        m_pNode->pos = m_Point;
    }
    m_pModel->schemeUpdated();
}

MoveNode::MoveNode(const QVariant &selection, const QVariant &nodes, AbstractSchemeModel *model, const QString &name, QUndoCommand *parent) :
        UndoScheme(model, name, parent) {

    QVariantList list = nodes.toList();
    for(auto it : selection.toList()) {
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
        AbstractSchemeModel::Node *node = m_pModel->node(m_Indices.at(i));
        positions.push_back(node->pos);
        node->pos = m_Points.at(i);
    }
    m_Points = positions;
    m_pModel->nodeMoved();
}

DeleteNodes::DeleteNodes(const QVariant &selection, AbstractSchemeModel *model, const QString &name, QUndoCommand *parent) :
        UndoScheme(model, name, parent) {

    for(QVariant it : selection.toList()) {
        m_Indices.push_back(it.toInt());
    }
}
void DeleteNodes::undo() {
    QVariantMap data = m_Document.toVariant().toMap();

    QVariantList nodes = data[NODES].toList();
    for(int i = 0; i < nodes.size(); ++i) {
        QVariantMap n = nodes[i].toMap();
        int32_t index = m_Indices.at(i);
        AbstractSchemeModel::Node *node = m_pModel->nodeCreate(n[TYPE].toString(), index);
        node->pos = QPoint(n[X].toInt(), n[Y].toInt());
        m_pModel->loadUserValues(node, n[VALUES].toMap());
    }

    QVariantList links = data[LINKS].toList();
    for(int i = 0; i < links.size(); ++i) {
        QVariantMap l = links[i].toMap();
        m_pModel->createLink(l[SENDER].toInt(), l[OPORT].toInt(), l[RECEIVER].toInt(), l[IPORT].toInt());
    }

    m_pModel->schemeUpdated();
}
void DeleteNodes::redo() {
    QVariantList links;

    AbstractSchemeModel::NodeList list;
    for(auto it : m_Indices) {
        AbstractSchemeModel::Node *node = m_pModel->node(it);
        list.push_back(node);
        for(auto l : m_pModel->findLinks(node)) {
            QVariantMap link;
            link[SENDER] = m_pModel->m_Nodes.indexOf(l->sender);
            link[OPORT] = (l->oport != nullptr) ? l->sender->list.indexOf(l->oport) : -1;
            link[RECEIVER] = m_pModel->m_Nodes.indexOf(l->receiver);
            link[IPORT] = (l->iport != nullptr) ? l->receiver->list.indexOf(l->iport) : -1;

            links.push_back(link);
        }
    }

    QVariantList nodes;
    for(auto it : list) {
        nodes.push_back(m_pModel->saveNode(it));
        m_pModel->nodeDelete(it);
    }

    QVariantMap data;
    data[NODES] = nodes;
    data[LINKS] = links;

    m_Document =  QJsonDocument::fromVariant(data);

    m_pModel->schemeUpdated();
}

PasteNodes::PasteNodes(const QString &data, int x, int y, AbstractSchemeModel *model, const QString &name, QUndoCommand *parent) :
        UndoScheme(model, name, parent),
        m_Document(QJsonDocument::fromJson(data.toLocal8Bit())),
        m_X(x),
        m_Y(y) {

}
void PasteNodes::undo() {
    for(auto it : m_List) {
        m_pModel->nodeDelete(m_pModel->node(it.toInt()));
    }
    m_pModel->schemeUpdated();
}
void PasteNodes::redo () {
    int maxX = INT_MIN;
    int maxY = INT_MIN;
    for(QJsonValue it : m_Document.array()) {
        QVariantMap n = it.toVariant().toMap();
        maxX = MAX(maxX, n[X].toInt());
        maxY = MAX(maxY, n[Y].toInt());
    }

    m_List.clear();
    for(QJsonValue it : m_Document.array()) {
        QVariantMap n = it.toVariant().toMap();

        int index = -1;
        AbstractSchemeModel::Node *node = m_pModel->nodeCreate(n[TYPE].toString(), index);
        int deltaX = maxX - n[X].toInt();
        int deltaY = maxY - n[Y].toInt();
        node->pos = QPoint(m_X + deltaX, m_Y + deltaY);
        m_pModel->loadUserValues(node, n[VALUES].toMap());

        m_List.push_back(index);
    }
    m_pModel->schemeUpdated();
}
