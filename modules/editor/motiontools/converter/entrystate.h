#ifndef ENTRYSTATE_H
#define ENTRYSTATE_H

#include <editor/graph/abstractnodegraph.h>

class EntryState : public GraphNode {
    Q_OBJECT

public:
    Vector2 defaultSize() const override;
    Vector4 color() const override;

    bool isState() const override;

};

#endif // ENTRYSTATE_H
