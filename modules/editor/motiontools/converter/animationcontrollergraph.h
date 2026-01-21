#ifndef ANIMATIONCONTROLLERGRAPH_H
#define ANIMATIONCONTROLLERGRAPH_H

#include <editor/graph/abstractnodegraph.h>

class AnimationControllerGraph : public AbstractNodeGraph {
    A_OBJECT(AnimationControllerGraph, AbstractNodeGraph, Editor)

public:
    AnimationControllerGraph();

    StringList nodeList() const override;

    Variant data() const;

private:
    GraphNode *fallbackRoot() override;

    void onNodesLoaded() override;

    GraphNode *nodeCreate(const TString &type, int &index) override;

    GraphLink *linkCreate(GraphNode *sender, NodePort *oport, GraphNode *receiver, NodePort *iport) override;

private:
    static StringList m_nodeTypes;

    GraphNode *m_entryState;

};

#endif // ANIMATIONCONTROLLERGRAPH_H
