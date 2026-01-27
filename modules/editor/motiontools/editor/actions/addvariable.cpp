#include "addvariable.h"

AddVariable::AddVariable(const TString &variableName, const Variant &variable, AnimationControllerGraph *graph, const TString &name, UndoCommand *group) :
        UndoCommand(name, group),
        m_variableName(variableName),
        m_variable(variable),
        m_graph(graph) {

}

void AddVariable::undo() {
    m_variable = m_graph->variable(m_variableName);
    m_graph->removeVariable(m_variableName);
    m_graph->variableChanged();
}

void AddVariable::redo() {
    m_graph->addVariable(m_variableName, m_variable);
    m_graph->variableChanged();
}
