#ifndef ANIMATIONCONTROLLERGRAPH_H
#define ANIMATIONCONTROLLERGRAPH_H

#include <editor/graph/abstractnodegraph.h>

class AnimationControllerGraph : public AbstractNodeGraph {
    Q_OBJECT

public:
    AnimationControllerGraph();

    void loadGraph(const QDomElement &parent) override;

    Variant object() const;

    StringList nodeList() const override;

private:
    void onNodesLoaded() override;

    GraphNode *nodeCreate(const TString &path, int &index) override;
    Link *linkCreate(GraphNode *sender, NodePort *oport, GraphNode *receiver, NodePort *iport) override;

protected:
    Variant data() const;

    GraphNode *m_entryState;

    QString m_path;

    StringList m_functions;

};

#endif // ANIMATIONCONTROLLERGRAPH_H
