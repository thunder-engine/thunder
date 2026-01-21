#ifndef ENTRYSTATE_H
#define ENTRYSTATE_H

#include <editor/graph/statenode.h>

class EntryState : public StateNode {
    A_OBJECT(EntryState, StateNode, Graph)

private:
    bool isRemovable() const override { return false; }

    Vector2 defaultSize() const override {
        return Vector2(170.0f, 40.0f);
    }

    Vector4 color() const override {
        return Vector4(0.22f, 0.46, 0.11f, 1.0f);
    }
};

#endif // ENTRYSTATE_H
