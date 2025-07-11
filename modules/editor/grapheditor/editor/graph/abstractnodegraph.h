#ifndef ABSTRACTNODEGRAPH_H
#define ABSTRACTNODEGRAPH_H

#include <QVariant>
#include <QJsonDocument>
#include <QDomDocument>

#include <stdint.h>

#include <editor/undomanager.h>

#include <editor/graph/graphnode.h>

class Texture;

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
    typedef std::list<Link *> LinkList;
    typedef std::list<GraphNode *> NodeList;

public:
    AbstractNodeGraph();

    AbstractNodeGraph(const AbstractNodeGraph &) { assert(false && "DONT EVER USE THIS"); }

    virtual GraphNode *nodeCreate(const std::string &path, int &index) = 0;
    virtual void nodeDelete(GraphNode *node);

    virtual Link *linkCreate(GraphNode *sender, NodePort *oport, GraphNode *receiver, NodePort *iport);
    virtual void linkDelete(NodePort *port);
    virtual void linkDelete(GraphNode *node);
    virtual void linkDelete(Link *link);

    const LinkList findLinks(const GraphNode *node) const;
    const LinkList findLinks(const NodePort *port) const;
    const Link *findLink(const GraphNode *node, const NodePort *port) const;

    virtual GraphNode *defaultNode() const;

    bool isSingleConnection(const NodePort *port) const;

    GraphNode *node(int index);
    Link *link(int index);

    int node(GraphNode *node) const;
    int link(Link *link) const;

    void load(const std::string &path);
    void save(const std::string &path);

    virtual std::list<std::string> nodeList() const;

    const NodeList &nodes() const;
    const LinkList &links() const;

    void reportMessage(GraphNode *node, const std::string &text);

signals:
    void graphUpdated();
    void graphLoaded();

    void messageReported(int node, const std::string &text);

    void menuVisible(bool visible);

protected:
    virtual void loadGraph(const QDomElement &parent);

    virtual void onNodesLoaded();

    virtual void saveGraph(QDomElement &parent, QDomDocument &xml) const;

    virtual GraphNode *fallbackRoot();

    QVariantList saveLinks(GraphNode *node) const;

    friend class DeleteNodes;

protected:
    LinkList m_links;
    NodeList m_nodes;

    uint32_t m_version;

};

#endif // ABSTRACTNODEGRAPH_H
