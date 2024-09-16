#ifndef STATENODE_H
#define STATENODE_H

#include <editor/graph/graphnode.h>

class NODEGRAPH_EXPORT StateNode : public GraphNode {
    Q_OBJECT

private:
    Widget *widget() override;

};

#endif // STATENODE_H
