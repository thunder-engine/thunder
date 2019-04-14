#ifndef ABSTRACTSCHEMEMODEL_H
#define ABSTRACTSCHEMEMODEL_H

#include <QImage>
#include <QVariant>

#include <string>
#include <stdint.h>
#include <cassert>

#include <variant.h>

#include "converters/converter.h"

using namespace std;

class QAbstractItemModel;

class AbstractSchemeModel : public IConverter {
    Q_OBJECT

public:
    struct Item {
        bool                        out;

        uint32_t                    type;

        QString                     name;

        int32_t                     pos;

        QVariant                    var;
    };
    typedef QList<Item *>           ItemList;

    struct Node {
        bool                        root;

        QString                     name;

        QString                     type;

        void                       *ptr;

        ItemList                    list;

        QPoint                      pos;

        QColor                      color;

        QImage                      cache;
    };
    typedef QList<Node *>           NodeList;

    struct Link {
        Node                       *sender;

        Item                       *oport;

        Node                       *receiver;

        Item                       *iport;

        void                       *ptr;
    };
    typedef QList<Link *>           LinkList;

public:
    AbstractSchemeModel();

    AbstractSchemeModel(const AbstractSchemeModel &) { assert(false && "DONT EVER USE THIS"); }

    virtual Node *createNode(const QString &path) = 0;
    virtual void deleteNode(Node *node);

    virtual void createLink(Node *sender, Item *oport, Node *receiver, Item *iport);
    virtual void deleteLink(Item *item, bool silent = false);

    const Link *findLink(const Node *node, const char *item) const;

    const Node *node(int index);
    const Link *link(int index);

    virtual void load(const QString &path);
    virtual void save(const QString &path);

    virtual QAbstractItemModel *components () const = 0;

    NodeList &getNodes() {
        return m_Nodes;
    }

    const LinkList &getLinks() {
        return m_Links;
    }

    Q_INVOKABLE void createNode(const QString &path, int x, int y);

    Q_INVOKABLE QVariant nodes() const;
    Q_INVOKABLE QVariant links() const;

    Q_INVOKABLE void moveNode(int index, int x, int y);
    Q_INVOKABLE void deleteNode(int index);

    Q_INVOKABLE void createLink(int sender, int oport, int receiver, int iport);
    Q_INVOKABLE void deleteLink(int index);

    Q_INVOKABLE void deleteLinksByNode(int index, int port);

signals:
    void schemeUpdated();

    void nodeMoved();

protected:
    virtual void loadUserValues(Node *node, const QVariantMap &values) = 0;
    virtual void saveUserValues(Node *node, QVariantMap &values) = 0;

protected:
    NodeList m_Nodes;
    LinkList m_Links;

    AbstractSchemeModel::Node *m_pRootNode;

    QVariantMap m_Data;
};

#endif // ABSTRACTSCHEMEMODEL_H
