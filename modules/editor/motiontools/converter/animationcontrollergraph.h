#ifndef ANIMATIONCONTROLLERGRAPH_H
#define ANIMATIONCONTROLLERGRAPH_H

#include <editor/graph/abstractnodegraph.h>

class AnimationControllerGraph : public AbstractNodeGraph {
    Q_OBJECT

public:
    AnimationControllerGraph();

    void load(const QString &path) Q_DECL_OVERRIDE;
    void save(const QString &path) Q_DECL_OVERRIDE;

    void loadGraph(const QVariantMap &data) Q_DECL_OVERRIDE;

    Variant object() const;

    QStringList nodeList() const Q_DECL_OVERRIDE;

private:
    GraphNode *createRoot() Q_DECL_OVERRIDE;
    GraphNode *nodeCreate(const QString &path, int &index) Q_DECL_OVERRIDE;
    Link *linkCreate(GraphNode *sender, NodePort *oport, GraphNode *receiver, NodePort *iport) Q_DECL_OVERRIDE;

    void loadUserValues(GraphNode *node, const QVariantMap &values) Q_DECL_OVERRIDE;
    void saveUserValues(GraphNode *node, QVariantMap &values) Q_DECL_OVERRIDE;

protected:
    Variant data() const;

    GraphNode *m_entry;
    QString m_path;

    QStringList m_functions;

};

#endif // ANIMATIONCONTROLLERGRAPH_H
