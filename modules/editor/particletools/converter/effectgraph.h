#ifndef SHADERGRAPH_H
#define SHADERGRAPH_H

#include <editor/graph/graphnode.h>
#include <editor/graph/abstractnodegraph.h>

class EffectRootNode;
class QMenu;

class EffectGraph : public AbstractNodeGraph {
    Q_OBJECT

public:
    EffectGraph();

    VariantMap data() const;

    Variant object() const;

    StringList nodeList() const override;

    GraphNode *defaultNode() const override;

    String modulePath(const String &name);

    StringList modules() const;

signals:
    void moduleChanged();

    void effectUpdated();

public slots:
    void onNodesLoaded() override;

private:
    void scanForFunctions();

    GraphNode *nodeCreate(const String &type, int &index) override;

private:
    StringList m_nodeTypes;

    std::map<String, String> m_exposedModules;

    EffectRootNode *m_rootNode;

};

#endif // SHADERGRAPH_H
