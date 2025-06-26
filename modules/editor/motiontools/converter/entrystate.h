#ifndef ENTRYSTATE_H
#define ENTRYSTATE_H

#include <editor/graph/statenode.h>

class EntryState : public StateNode {
    A_OBJECT(EntryState, StateNode, Graph)

private:
    bool isRemovable() const override { return false; }

    Vector2 defaultSize() const override;
    Vector4 color() const override;

};

#endif // ENTRYSTATE_H
