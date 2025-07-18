#ifndef SHADERGRAPH_H
#define SHADERGRAPH_H

#include <editor/graph/abstractnodegraph.h>

class EffectRootNode;

class EffectGraph : public AbstractNodeGraph {
    A_OBJECT(EffectGraph, AbstractNodeGraph, Editor)

    A_METHODS(
        A_SIGNAL(EffectGraph::moduleChanged),
        A_SIGNAL(EffectGraph::effectUpdated)
    )

public:
    EffectGraph();

    VariantMap data() const;

    Variant object() const;

    StringList nodeList() const override;

    GraphNode *defaultNode() const override;

    TString modulePath(const TString &name);

    StringList modules() const;

signals:
    void moduleChanged();
    void effectUpdated();

private:
    void onNodesLoaded() override;

    void scanForFunctions();

    GraphNode *nodeCreate(const TString &type, int &index) override;

private:
    StringList m_nodeTypes;

    std::map<TString, TString> m_exposedModules;

    EffectRootNode *m_rootNode;

};

#endif // SHADERGRAPH_H
