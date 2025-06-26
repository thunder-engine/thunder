#ifndef ANIMATIONCONTROLLERGRAPH_H
#define ANIMATIONCONTROLLERGRAPH_H

#include <editor/graph/abstractnodegraph.h>

class AnimationControllerGraph : public AbstractNodeGraph {
    Q_OBJECT

public:
    AnimationControllerGraph();

    void loadGraph(const QDomElement &parent) override;

    Variant object() const;

    std::list<std::string> nodeList() const override;

private:
    void onNodesLoaded() override;

    GraphNode *nodeCreate(const std::string &path, int &index) override;
    Link *linkCreate(GraphNode *sender, NodePort *oport, GraphNode *receiver, NodePort *iport) override;

protected:
    Variant data() const;

    GraphNode *m_entryState;

    QString m_path;

    std::list<std::string> m_functions;

};

#endif // ANIMATIONCONTROLLERGRAPH_H
