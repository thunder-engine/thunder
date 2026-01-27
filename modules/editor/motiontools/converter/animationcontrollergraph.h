#ifndef ANIMATIONCONTROLLERGRAPH_H
#define ANIMATIONCONTROLLERGRAPH_H

#include <editor/graph/abstractnodegraph.h>

class AnimationControllerGraph : public AbstractNodeGraph {
    A_OBJECT(AnimationControllerGraph, AbstractNodeGraph, Editor)

    A_METHODS(
        A_SIGNAL(AnimationControllerGraph::variableChanged)
    )

public:
    AnimationControllerGraph();

    StringList variables() const;
    Variant variable(const TString &name);

    void addVariable(const TString &name, Variant &value);
    void removeVariable(const TString &name);

    StringList nodeList() const override;

    Variant data() const;

public: // signals
    void variableChanged();

private:
    GraphNode *fallbackRoot() override;

    void onNodesLoaded() override;

    GraphNode *nodeCreate(const TString &type, int &index) override;

    GraphLink *linkCreate(GraphNode *sender, NodePort *oport, GraphNode *receiver, NodePort *iport) override;
    GraphLink *linkCreate() override;

    void loadGraph(const pugi::xml_node &graph) override;
    void saveGraph(pugi::xml_node &graph) const override;

private:
    static StringList m_nodeTypes;

    std::map<TString, Variant> m_variables;

    GraphNode *m_entryState;

};

#endif // ANIMATIONCONTROLLERGRAPH_H
