#ifndef ABSTRACTNODEGRAPH_H
#define ABSTRACTNODEGRAPH_H

#include <stdint.h>

#include <editor/graph/graphnode.h>

class NODEGRAPH_EXPORT AbstractNodeGraph : public Object {
    A_OBJECT(AbstractNodeGraph, Object, Editor)

    A_METHODS(
        A_SIGNAL(AbstractNodeGraph::graphUpdated),
        A_SIGNAL(AbstractNodeGraph::graphLoaded)
    )

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

    virtual GraphNode *nodeCreate(const TString &path, int &index);
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

    GraphNode *node(int index) const;
    Link *link(int index) const;

    int node(const GraphNode *node) const;
    int link(const Link *link) const;

    void load(const TString &path);
    void save(const TString &path);

    virtual StringList nodeList() const;

    const NodeList &nodes() const;
    const LinkList &links() const;

    void graphUpdated();
    void graphLoaded();

protected:
    virtual void loadGraph(const pugi::xml_node &parent);

    virtual void onNodesLoaded();

    virtual void saveGraph(pugi::xml_node &parent) const;

    virtual GraphNode *fallbackRoot();

    void saveLinks(GraphNode *node, pugi::xml_node &parent) const;

    friend class DeleteNodes;

protected:
    LinkList m_links;
    NodeList m_nodes;

    uint32_t m_version;

};

#endif // ABSTRACTNODEGRAPH_H
