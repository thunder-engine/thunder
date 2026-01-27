#ifndef RENAMEVARIABLE_H
#define RENAMEVARIABLE_H

#include "animationcontrollergraph.h"

#include <editor/undostack.h>

class RenameVariable : public UndoCommand {
public:
    RenameVariable(const TString &oldName, const TString &newName, AnimationControllerGraph *graph, const TString &name = "Rename Variable", UndoCommand *group = nullptr);

    void undo() override;
    void redo() override;

protected:
    TString m_oldName;
    TString m_newName;

    AnimationControllerGraph *m_graph;

};

#endif // RENAMEVARIABLE_H
