#ifndef ANIMATIONCONTROLLERGRAPH_H
#define ANIMATIONCONTROLLERGRAPH_H

#include <editor/graph/abstractnodegraph.h>

class AnimationControllerGraph : public AbstractNodeGraph {
public:
    AnimationControllerGraph();

    StringList nodeList() const override;

    Variant data() const;

private:
    GraphNode *fallbackRoot() override;

    void onNodesLoaded() override;

    GraphNode *nodeCreate(const TString &type, int &index) override;
    Link *linkCreate(GraphNode *sender, NodePort *oport, GraphNode *receiver, NodePort *iport) override;

protected:


    GraphNode *m_entryState;

    TString m_path;

    static StringList m_nodeTypes;

};

#endif // ANIMATIONCONTROLLERGRAPH_H
