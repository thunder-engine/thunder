#ifndef ANIMATIONCONTROLLERGRAPH_H
#define ANIMATIONCONTROLLERGRAPH_H

#include <editor/graph/abstractnodegraph.h>

class AnimationControllerGraph : public AbstractNodeGraph {
    Q_OBJECT

public:
    AnimationControllerGraph();

    void loadGraphV0(const QVariantMap &data) override;
    void loadGraphV11(const QDomElement &parent) override;

    Variant object() const;

    QStringList nodeList() const override;

private:
    void onNodesLoaded() override;

    GraphNode *nodeCreate(const QString &path, int &index) override;
    Link *linkCreate(GraphNode *sender, NodePort *oport, GraphNode *receiver, NodePort *iport) override;

protected:
    Variant data() const;

    GraphNode *m_entryState;

    QString m_path;

    QStringList m_functions;

};

#endif // ANIMATIONCONTROLLERGRAPH_H
