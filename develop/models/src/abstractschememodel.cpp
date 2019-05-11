#include "abstractschememodel.h"

#include <QJsonDocument>
#include <QFile>

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

void AbstractSchemeModel::deleteNode(Node *node) {
    if(node == m_pRootNode) {
        return;
    }
    bool result = false;
    auto it = m_Nodes.begin();
    while(it != m_Nodes.end()) {
        if(*it == node) {
            for(Item *item : node->list) {
                deleteLink(item, true);
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
            result  = true;
            break;
        } else {
            ++it;
        }
    }
    if(result) {
        emit schemeUpdated();
    }
}

void AbstractSchemeModel::createLink(Node *sender, Item *oport, Node *receiver, Item *iport) {
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
                deleteLink(iport);
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

void AbstractSchemeModel::deleteLink(Item *item, bool silent) {
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

const AbstractSchemeModel::Link *AbstractSchemeModel::findLink(const Node *node, const char *item) const {
    for(const auto it : m_Links) {
        QString str;
        str.compare(item);
        if(it->receiver == node && it->iport->name.compare(item) == 0) {
            return it;
        }
    }
    return nullptr;
}

const AbstractSchemeModel::Node *AbstractSchemeModel::node(int index) {
    return m_Nodes.at(index);
}

const AbstractSchemeModel::Link *AbstractSchemeModel::link(int index) {
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
        Node *node = createNode(n[TYPE].toString());
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
            QVariantMap node;
            node[TYPE] = it->type;
            node[X] = it->pos.x();
            node[Y] = it->pos.y();

            QVariantMap values;
            saveUserValues(it, values);
            node[VALUES]    = values;

            nodes.push_back(node);
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

void AbstractSchemeModel::createNode(const QString &path, int x, int y) {
    Node *node = createNode(path);
    if(node) {
        node->pos = QPoint(x, y);
    }
    emit schemeUpdated();
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

        createLink(snd, op, rcv, ip);
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
            deleteLink(node->list.at(port));
        }
    }
}

void AbstractSchemeModel::moveNode(int index, int x, int y) {
    m_Nodes[index]->pos = QPoint(x, y);
    emit nodeMoved();
}

void AbstractSchemeModel::deleteNodes(QVariant list) {
    NodeList toDelete;
    for(QVariant it : list.toList()) {
        toDelete.push_back(m_Nodes.at(it.toInt()));
    }
    for(Node *it : toDelete) {
        deleteNode(it);
    }
}
