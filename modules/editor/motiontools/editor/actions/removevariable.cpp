#include "removevariable.h"

RemoveVariable::RemoveVariable(const TString &variableName, AnimationControllerGraph *graph, const TString &name, UndoCommand *group) :
        UndoCommand(name, group),
        m_variableName(variableName),
        m_graph(graph) {

}

void RemoveVariable::undo() {
    m_graph->addVariable(m_variableName, m_variable);
    m_graph->variableChanged();
}

void RemoveVariable::redo() {
    m_variable = m_graph->variable(m_variableName);
    m_graph->removeVariable(m_variableName);
    m_graph->variableChanged();
}
