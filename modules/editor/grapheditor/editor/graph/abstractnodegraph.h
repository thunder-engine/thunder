#ifndef ABSTRACTNODEGRAPH_H
#define ABSTRACTNODEGRAPH_H

#include <stdint.h>

#include <editor/graph/graphnode.h>

class NODEGRAPH_EXPORT GraphLink : public Object {
    A_OBJECT(GraphLink, Object, Graph)

public:
    virtual void toXml(pugi::xml_node &element) {}
    virtual void fromXml(const pugi::xml_node &element) {}

public:
    GraphNode *sender = nullptr;

    NodePort *oport = nullptr;

    GraphNode *receiver = nullptr;

    NodePort *iport = nullptr;

    void *ptr = nullptr;

};

class NODEGRAPH_EXPORT AbstractNodeGraph : public Object {
    A_OBJECT(AbstractNodeGraph, Object, Editor)

    A_METHODS(
        A_SIGNAL(AbstractNodeGraph::graphUpdated),
        A_SIGNAL(AbstractNodeGraph::graphLoaded)
    )

public:
    typedef std::list<GraphLink *> LinkList;
    typedef std::list<GraphNode *> NodeList;

public:
    AbstractNodeGraph();

    virtual GraphNode *nodeCreate(const TString &path, int &index);
    virtual void nodeDelete(GraphNode *node);

    virtual GraphLink *linkCreate(GraphNode *sender, NodePort *oport, GraphNode *receiver, NodePort *iport);
    virtual void linkDelete(NodePort *port);
    virtual void linkDelete(GraphNode *node);
    virtual void linkDelete(GraphLink *link);

    const LinkList findLinks(const GraphNode *node) const;
    const LinkList findLinks(const NodePort *port) const;
    const GraphLink *findLink(const GraphNode *node, const NodePort *port) const;

    virtual GraphNode *defaultNode() const;

    bool isSingleConnection(const NodePort *port) const;

    GraphNode *node(int index) const;
    GraphLink *link(int index) const;

    int node(const GraphNode *node) const;
    int link(const GraphLink *link) const;

    void load(const TString &path);
    void save(const TString &path);

    virtual StringList nodeList() const;

    const NodeList &nodes() const;
    const LinkList &links() const;

    void graphUpdated();
    void graphLoaded();

protected:
    virtual GraphLink *linkCreate();

    virtual void loadGraph(const pugi::xml_node &parent);

    virtual void onNodesLoaded();

    virtual void saveGraph(pugi::xml_node &parent) const;

    virtual GraphNode *fallbackRoot();

    void saveLinks(GraphNode *node, pugi::xml_node &parent) const;
    void saveLink(GraphLink *link, pugi::xml_node &parent) const;

    int32_t loadNode(pugi::xml_node &element);
    int32_t loadLink(pugi::xml_node &element);

protected:
    friend class DeleteNodes;
    friend class DeleteLinks;

    LinkList m_links;
    NodeList m_nodes;

    uint32_t m_version;

};

#endif // ABSTRACTNODEGRAPH_H
