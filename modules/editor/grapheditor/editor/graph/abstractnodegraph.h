#ifndef ABSTRACTNODEGRAPH_H
#define ABSTRACTNODEGRAPH_H

#include <QVariant>
#include <QJsonDocument>

#include <stdint.h>

#include <editor/undomanager.h>

#include <editor/graph/graphnode.h>

class NODEGRAPH_EXPORT AbstractNodeGraph : public QObject {
    Q_OBJECT

public:
    struct Link {
        GraphNode *sender;

        NodePort *oport;

        GraphNode *receiver;

        NodePort *iport;

        void *ptr;
    };
    typedef QList<Link *> LinkList;
    typedef QList<GraphNode *> NodeList;

public:
    AbstractNodeGraph();

    AbstractNodeGraph(const AbstractNodeGraph &) { assert(false && "DONT EVER USE THIS"); }

    virtual GraphNode *nodeCreate(const QString &path, int &index) = 0;
    virtual void nodeDelete(GraphNode *node);

    virtual Link *linkCreate(GraphNode *sender, NodePort *oport, GraphNode *receiver, NodePort *iport);
    virtual void linkDelete(NodePort *port);
    virtual void linkDelete(GraphNode *node);
    virtual void linkDelete(Link *link);

    const LinkList findLinks(const GraphNode *node) const;
    const LinkList findLinks(const NodePort *port) const;
    const Link *findLink(const GraphNode *node, const NodePort *port) const;

    bool isSingleConnection(const NodePort *port) const;

    GraphNode *node(int index);
    Link *link(int index);

    int node(GraphNode *node) const;
    int link(Link *link) const;

    virtual void load(const QString &path);
    virtual void save(const QString &path);

    virtual QStringList nodeList() const;

    const NodeList &nodes() const;
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

    void reportMessage(GraphNode *node, const QString &text);

signals:
    void graphUpdated();

    void nodeMoved();

    void messageReported(int node, const QString &text);

protected:
    virtual void loadUserValues(GraphNode *node, const QVariantMap &values) = 0;
    virtual void saveUserValues(GraphNode *node, QVariantMap &values) = 0;

    virtual GraphNode *createRoot() = 0;

    QVariant saveNode(GraphNode *node);

    friend class PasteNodes;
    friend class DeleteNodes;

protected:
    LinkList m_links;
    NodeList m_nodes;

    GraphNode *m_rootNode;

    QVariantMap m_data;
};

class UndoScheme : public UndoCommand {
public:
    UndoScheme(AbstractNodeGraph *graph, const QString &name, QUndoCommand *parent = nullptr) :
            UndoCommand(name, graph, parent) {
        m_graph = graph;
    }
protected:
    AbstractNodeGraph *m_graph;
};

class CreateNode : public UndoScheme {
public:
    CreateNode(const QString &path, int x, int y, AbstractNodeGraph *graph, int node = -1, int port = -1, bool out = false, const QString &name = QObject::tr("Create Node"), QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;
private:
    GraphNode *m_node;
    QString m_path;
    int m_linkIndex;
    int m_fromNode;
    int m_fromPort;
    bool m_out;
    QPoint m_point;
};

class MoveNode : public UndoScheme {
public:
    MoveNode(const QVariant &selection, const QVariant &nodes, AbstractNodeGraph *graph, const QString &name = QObject::tr("Move Node"), QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;
private:
    QList<int> m_indices;
    QList<QPoint> m_points;
};

class DeleteNodes : public UndoScheme {
public:
    DeleteNodes(const QVariant &selection, AbstractNodeGraph *graph, const QString &name = QObject::tr("Delete Node"), QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;
private:
    QList<int> m_indices;
    QJsonDocument m_document;
};

class PasteNodes : public UndoScheme {
public:
    PasteNodes(const QString &data, int x, int y, AbstractNodeGraph *graph, const QString &name = QObject::tr("Paste Node"), QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

    QVariant list() const { return m_list; }

private:
    QJsonDocument m_document;
    int32_t m_x;
    int32_t m_y;
    QVariantList m_list;
};

class CreateLink : public UndoScheme {
public:
    CreateLink(int sender, int oport, int receiver, int iport, AbstractNodeGraph *graph, const QString &name = QObject::tr("Create Link"), QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    int m_sender;
    int m_oPort;
    int m_receiver;
    int m_iPort;

    int m_index;
};

class DeleteLink : public UndoScheme {
public:
    DeleteLink(int index, AbstractNodeGraph *graph, const QString &name = QObject::tr("Delete Link"), QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    int m_sender;
    int m_oPort;
    int m_receiver;
    int m_iPort;

    int m_index;
};

class DeleteLinksByPort : public UndoScheme {
public:
    DeleteLinksByPort(int node, int port, AbstractNodeGraph *graph, const QString &name = QObject::tr("Delete Links"), QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    int m_node;
    int m_port;

    QVariantList m_links;
};

#endif // ABSTRACTNODEGRAPH_H
