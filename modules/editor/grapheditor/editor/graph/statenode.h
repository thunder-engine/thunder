#ifndef STATENODE_H
#define STATENODE_H

#include <editor/graph/graphnode.h>

class NODEGRAPH_EXPORT StateNode : public GraphNode {
    A_OBJECT(StateNode, GraphNode, Graph)

protected:
    void updateName();

    Widget *widget() override;

};

#endif // STATENODE_H
