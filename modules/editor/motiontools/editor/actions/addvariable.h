#ifndef ADDVARIABLE_H
#define ADDVARIABLE_H

#include "animationcontrollergraph.h"

#include <editor/undostack.h>

class AddVariable : public UndoCommand {
public:
    AddVariable(const TString &variableName, const Variant &variable, AnimationControllerGraph *graph, const TString &name = "New Variable", UndoCommand *group = nullptr);

    void undo() override;
    void redo() override;

protected:
    TString m_variableName;

    Variant m_variable;

    AnimationControllerGraph *m_graph;

};

#endif // ADDVARIABLE_H
