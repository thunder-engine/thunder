#ifndef ABSTRACTNODEGRAPH_H
#define ABSTRACTNODEGRAPH_H

#include <QVariant>
#include <QJsonDocument>
#include <QDomDocument>

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

    GraphNode *rootNode() const;

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
    const LinkList &links() const;

    void createNode(const QString &path, int x, int y);
    void deleteNodes(const vector<int32_t> &selection);
    void copyNodes(const vector<int32_t> &selection);
    QVariant pasteNodes(int x, int y);
    void createAndLink(const QString &path, int x, int y, int node, int port, bool out);

    void createLink(int sender, int oport, int receiver, int iport);
    void deleteLink(int index);

    void deleteLinksByPort(int node, int port);

    void reportMessage(GraphNode *node, const QString &text);

signals:
    void graphUpdated();

    void messageReported(int node, const QString &text);

protected:
    virtual void loadUserValues(GraphNode *node, const QVariantMap &values) = 0;
    virtual void saveUserValues(GraphNode *node, QVariantMap &values) = 0;

    virtual GraphNode *createRoot() = 0;

    QVariantMap loadXmlData(const QByteArray &data);
    QVariantMap loadXmlMap(const QDomElement &parent);
    QVariantList loadXmlList(const QDomElement &parent);

    QByteArray saveXmlData(const QVariantMap &data);
    void saveXmlMap(const QVariantMap &data, QDomDocument &xml, QDomElement &parent);
    void saveXmlList(const QVariantList &data, QDomDocument &xml, QDomElement &parent);

    void loadGraph(const QVariantMap &data);

    QVariantMap saveNode(GraphNode *node);
    QVariantList saveLinks(GraphNode *node);

    friend class PasteNodes;
    friend class DeleteNodes;

protected:
    LinkList m_links;
    NodeList m_nodes;

    GraphNode *m_rootNode;

    QVariantMap m_data;
};

class UndoGraph : public UndoCommand {
public:
    UndoGraph(AbstractNodeGraph *graph, const QString &name, QUndoCommand *parent = nullptr) :
            UndoCommand(name, graph, parent) {
        m_graph = graph;
    }
protected:
    AbstractNodeGraph *m_graph;
};

class CreateNode : public UndoGraph {
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
    Vector2 m_point;
};

class DeleteNodes : public UndoGraph {
public:
    DeleteNodes(const vector<int32_t> &selection, AbstractNodeGraph *graph, const QString &name = QObject::tr("Delete Node"), QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;
private:
    vector<int32_t> m_indices;
    QVariantMap m_document;
};

class PasteNodes : public UndoGraph {
public:
    PasteNodes(const QString &data, int x, int y, AbstractNodeGraph *graph, const QString &name = QObject::tr("Paste Node"), QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

    QVariant list() const { return m_list; }

private:
    QJsonDocument m_document;
    QVariantList m_list;

    int32_t m_x;
    int32_t m_y;
};

class CreateLink : public UndoGraph {
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

class DeleteLink : public UndoGraph {
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

class DeleteLinksByPort : public UndoGraph {
public:
    DeleteLinksByPort(int node, int port, AbstractNodeGraph *graph, const QString &name = QObject::tr("Delete Links"), QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    int m_node;
    int m_port;

    struct Link {
        int sender;
        int oport;
        int receiver;
        int iport;
    };

    list<Link> m_links;
};

#endif // ABSTRACTNODEGRAPH_H
