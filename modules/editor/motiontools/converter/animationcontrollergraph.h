#ifndef ANIMATIONCONTROLLERGRAPH_H
#define ANIMATIONCONTROLLERGRAPH_H

#include <editor/graph/abstractnodegraph.h>

class AnimationControllerGraph : public AbstractNodeGraph {
    Q_OBJECT

public:
    AnimationControllerGraph();

    void loadGraphV0(const QVariantMap &data) override;
    void loadGraphV11(const QDomElement &parent) override;
    void saveGraph(QDomElement parent, QDomDocument xml) const override;

    Variant object() const;

    QStringList nodeList() const Q_DECL_OVERRIDE;

private:
    GraphNode *createRoot() Q_DECL_OVERRIDE;
    GraphNode *nodeCreate(const QString &path, int &index) override;
    Link *linkCreate(GraphNode *sender, NodePort *oport, GraphNode *receiver, NodePort *iport) override;

    void loadUserValues(GraphNode *node, const QVariantMap &values) override;
    void saveUserValues(GraphNode *node, QVariantMap &values) const override;

protected:
    Variant data() const;

    GraphNode *m_entry;
    QString m_path;

    QStringList m_functions;

};

#endif // ANIMATIONCONTROLLERGRAPH_H
