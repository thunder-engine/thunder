#ifndef ENTRYSTATE_H
#define ENTRYSTATE_H

#include <editor/graph/statenode.h>

class EntryState : public StateNode {
    Q_OBJECT

private:
    Vector2 defaultSize() const override;
    Vector4 color() const override;

};

#endif // ENTRYSTATE_H
