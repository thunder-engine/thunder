#include "renamevariable.h"

RenameVariable::RenameVariable(const TString &oldName, const TString &newName, AnimationControllerGraph *graph, const TString &name, UndoCommand *group) :
        UndoCommand(name, group),
        m_oldName(oldName),
        m_newName(newName),
        m_graph(graph) {

}

void RenameVariable::undo() {
    redo();
}

void RenameVariable::redo() {
    Variant variable = m_graph->variable(m_oldName);
    m_graph->removeVariable(m_oldName);
    m_graph->addVariable(m_newName, variable);
    m_graph->variableChanged();

    std::swap(m_oldName, m_newName);
}
