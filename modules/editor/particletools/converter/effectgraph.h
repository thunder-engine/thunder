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

    QStringList nodeList() const override;

    EffectRootNode *rootNode();

    QString modulePath(QString name);

signals:
    void moduleChanged();

    void effectUpdated();

public slots:
    void showFunctionsMenu();

private:
    void scanForFunctions();

    GraphNode *nodeCreate(const QString &path, int &index) override;

    void onNodesLoaded() override;

private:
    QStringList m_nodeTypes;

    std::map<QString, QString> m_exposedModules;

    EffectRootNode *m_rootNode;

    QMenu *m_menu;

};

#endif // SHADERGRAPH_H
