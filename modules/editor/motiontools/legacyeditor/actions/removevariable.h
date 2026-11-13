#ifndef REMOVEVARIABLE_H
#define REMOVEVARIABLE_H

#include "animationcontrollergraph.h"

#include <editor/undostack.h>

class RemoveVariable : public UndoCommand {
public:
    RemoveVariable(const TString &variableName, AnimationControllerGraph *graph, const TString &name = "Delete Variable", UndoCommand *group = nullptr);

    void undo() override;
    void redo() override;

protected:
    TString m_variableName;

    Variant m_variable;

    AnimationControllerGraph *m_graph;

};

#endif // REMOVEVARIABLE_H
