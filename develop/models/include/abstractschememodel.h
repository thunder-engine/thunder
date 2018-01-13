#ifndef ABSTRACTSCHEMEMODEL_H
#define ABSTRACTSCHEMEMODEL_H

#include <QImage>
#include <QVariant>

#include <string>
#include <stdint.h>
#include <cassert>

#include <avariant.h>

using namespace std;

class QAbstractItemModel;

class AbstractSchemeModel : public QObject {
    Q_OBJECT

public:
    struct Item {
        /// Derection of item
        bool                        out;
        /// Type of item
        uint32_t                    type;
        /// Displaying name of item.
        QString                     name;
        /// Displaying position of item.
        uint32_t                    pos;

        QVariant                    var;
    };
    typedef list<Item *>            ItemList;

    struct Node {
        bool                        root;

        QString                     name;

        QString                     type;

        void                       *ptr;

        ItemList                    list;

        QPoint                      pos;

        QImage                      cache;
    };
    typedef QList<Node *>           NodeList;

    struct Link {
        Node                       *sender;

        Item                       *sitem;

        Node                       *receiver;

        Item                       *ritem;
    };
    typedef QList<Link *>           LinkList;

public:
    AbstractSchemeModel             () {}

    AbstractSchemeModel             (const AbstractSchemeModel &) { assert(false && "DONT EVER USE THIS"); }

    virtual Node                   *createNode              (const QString &path) = 0;
    virtual void                    deleteNode              (Node *node) = 0;

    virtual void                    createLink              (Node *node1, Item *item1, Node *node2, Item *item2) = 0;
    virtual void                    deleteLink              (Item *item, bool silent = false) = 0;

    virtual QAbstractItemModel     *components              () const = 0;

    NodeList                       &nodes                   () {
        return m_Nodes;
    }

    const LinkList                 &links                   () const {
        return m_Links;
    }

signals:
    void                            schemeUpdated           ();

protected:
    NodeList                        m_Nodes;
    LinkList                        m_Links;
};

#endif // ABSTRACTSCHEMEMODEL_H
