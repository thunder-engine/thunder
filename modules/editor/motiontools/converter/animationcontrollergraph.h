#ifndef ANIMATIONCONTROLLERGRAPH_H
#define ANIMATIONCONTROLLERGRAPH_H

#include <editor/graph/abstractnodegraph.h>

class AnimationControllerGraph : public AbstractNodeGraph {
public:
    AnimationControllerGraph();

    Variant object() const;

    StringList nodeList() const override;

private:
    GraphNode *fallbackRoot() override;

    void onNodesLoaded() override;

    GraphNode *nodeCreate(const TString &type, int &index) override;
    Link *linkCreate(GraphNode *sender, NodePort *oport, GraphNode *receiver, NodePort *iport) override;

protected:
    Variant data() const;

    GraphNode *m_entryState;

    TString m_path;

    StringList m_nodeTypes;

};

#endif // ANIMATIONCONTROLLERGRAPH_H
