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

    std::list<std::string> nodeList() const override;

    GraphNode *defaultNode() const override;

    std::string modulePath(const std::string &name);

    std::list<std::string> modules() const;

signals:
    void moduleChanged();

    void effectUpdated();

public slots:
    void onNodesLoaded() override;

private:
    void scanForFunctions();

    GraphNode *nodeCreate(const std::string &type, int &index) override;

private:
    std::list<std::string> m_nodeTypes;

    std::map<std::string, std::string> m_exposedModules;

    EffectRootNode *m_rootNode;

};

#endif // SHADERGRAPH_H
