#ifndef ABSTRACTSCHEMEMODEL_H
#define ABSTRACTSCHEMEMODEL_H

#include <QImage>
#include <QVariant>
#include <QJsonDocument>

#include <string>
#include <stdint.h>
#include <cassert>

#include <variant.h>

#include <editor/assetconverter.h>

#include "undomanager.h"

using namespace std;

class QAbstractItemModel;

class AbstractSchemeModel;

class AbstractSchemeModel : public AssetConverter {
    Q_OBJECT

public:
    struct Port {
        bool                        out;

        uint32_t                    type;

        QString                     name;

        int32_t                     pos;

        QVariant                    var;
    };
    typedef QList<Port *>           PortList;

    struct Node {
        bool                        root;

        QString                     name;

        QString                     type;

        void                       *ptr;

        PortList                    list;

        QPoint                      pos;

        QColor                      color;

        QImage                      cache;
    };
    typedef QList<Node *>           NodeList;

    struct Link {
        Node                       *sender;

        Port                       *oport;

        Node                       *receiver;

        Port                       *iport;

        void                       *ptr;
    };
    typedef QList<Link *>           LinkList;

public:
    AbstractSchemeModel();

    AbstractSchemeModel(const AbstractSchemeModel &) { assert(false && "DONT EVER USE THIS"); }

    virtual Node *nodeCreate(const QString &path, int &index) = 0;
    virtual void nodeDelete(Node *node);

    virtual Link *linkCreate(Node *sender, Port *oport, Node *receiver, Port *iport);
    virtual void linkDelete(Port *item);
    virtual void linkDelete(Node *node);
    virtual void linkDelete(Link *link);

    const LinkList findLinks(const Node *node) const;
    const LinkList findLinks(const Port *node) const;
    const Link *findLink(const Node *node, const char *item) const;

    Node *node(int index);
    Link *link(int index);

    int node(Node *node) const;
    int link(Link *link) const;

    virtual void load(const QString &path);
    virtual void save(const QString &path);

    virtual QAbstractItemModel *components() const = 0;

    Q_INVOKABLE QVariant nodes() const;
    Q_INVOKABLE QVariant links() const;

    Q_INVOKABLE void createNode(const QString &path, int x, int y);
    Q_INVOKABLE void moveNode(const QVariant selection, const QVariant nodes);
    Q_INVOKABLE void deleteNodes(QVariant list);
    Q_INVOKABLE void copyNodes(QVariant list);
    Q_INVOKABLE QVariant pasteNodes(int x, int y);
    Q_INVOKABLE void createAndLink(const QString &path, int x, int y, int node, int port, bool out);

    Q_INVOKABLE void createLink(int sender, int oport, int receiver, int iport);
    Q_INVOKABLE void deleteLink(int index);

    Q_INVOKABLE void deleteLinksByPort(int node, int port);

    void reportMessage(Node *node, const QString &text);

signals:
    void schemeUpdated();

    void nodeMoved();

    void messageReported(int node, const QString &text);

protected:
    virtual void loadUserValues(Node *node, const QVariantMap &values) = 0;
    virtual void saveUserValues(Node *node, QVariantMap &values) = 0;

    QVariant saveNode(Node *node);

    friend class PasteNodes;
    friend class DeleteNodes;

protected:
    NodeList m_Nodes;
    LinkList m_Links;

    AbstractSchemeModel::Node *m_pRootNode;

    QVariantMap m_Data;
};

class UndoScheme : public UndoCommand {
public:
    UndoScheme(AbstractSchemeModel *model, const QString &name, QUndoCommand *parent = nullptr) :
            UndoCommand(name, model, parent) {
        m_pModel = model;
    }
protected:
    AbstractSchemeModel *m_pModel;
};

class CreateNode : public UndoScheme {
public:
    CreateNode(const QString &path, int x, int y, AbstractSchemeModel *model, int node = -1, int port = -1, bool out = false, const QString &name = QObject::tr("Create Node"), QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;
private:
    AbstractSchemeModel::Node *m_pNode;
    QString m_Path;
    int m_Index;
    int m_Node;
    int m_Port;
    bool m_Out;
    QPoint m_Point;
};

class MoveNode : public UndoScheme {
public:
    MoveNode(const QVariant &selection, const QVariant &nodes, AbstractSchemeModel *model, const QString &name = QObject::tr("Move Node"), QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;
private:
    QList<int> m_Indices;
    QList<QPoint> m_Points;
};

class DeleteNodes : public UndoScheme {
public:
    DeleteNodes(const QVariant &selection, AbstractSchemeModel *model, const QString &name = QObject::tr("Delete Node"), QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;
private:
    QList<int> m_Indices;
    QJsonDocument m_Document;
};

class PasteNodes : public UndoScheme {
public:
    PasteNodes(const QString &data, int x, int y, AbstractSchemeModel *model, const QString &name = QObject::tr("Paste Node"), QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

    QVariant list() const { return m_List; }

private:
    QJsonDocument m_Document;
    int32_t m_X;
    int32_t m_Y;
    QVariantList m_List;
};

class CreateLink : public UndoScheme {
public:
    CreateLink(int sender, int oport, int receiver, int iport, AbstractSchemeModel *model, const QString &name = QObject::tr("Create Link"), QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    int m_Sender;
    int m_OPort;
    int m_Receiver;
    int m_IPort;

    int m_Index;
};

class DeleteLink : public UndoScheme {
public:
    DeleteLink(int index, AbstractSchemeModel *model, const QString &name = QObject::tr("Delete Link"), QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    int m_Sender;
    int m_OPort;
    int m_Receiver;
    int m_IPort;

    int m_Index;
};

class DeleteLinksByPort : public UndoScheme {
public:
    DeleteLinksByPort(int node, int port, AbstractSchemeModel *model, const QString &name = QObject::tr("Delete Links"), QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    int m_Node;
    int m_Port;

    QVariantList m_Links;
};

#endif // ABSTRACTSCHEMEMODEL_H
