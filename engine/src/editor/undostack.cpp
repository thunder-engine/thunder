#include "undostack.h"

UndoCommand::UndoCommand(const TString &text, UndoCommand *parent) :
        m_text(text) {

    if(parent) {
        parent->m_childs.push_back(this);
    }
}

UndoCommand::~UndoCommand() {
    for(auto it : m_childs) {
        delete it;
    }
}

TString UndoCommand::text() const {
    return m_text;
}

void UndoCommand::undo() {
    for(int i = m_childs.size(); i >= 0; i--) {
        m_childs[i]->undo();
    }
}

void UndoCommand::redo() {
    for(auto it : m_childs) {
        it->redo();
    }
}

int UndoCommand::childCount() const {
    return m_childs.size();
}

UndoStack::UndoStack() :
        m_currentIndex(-1),
        m_cleanIndex(-1) {

}

UndoStack::~UndoStack() {
    clear();
}

void UndoStack::push(UndoCommand *cmd) {
    cmd->redo();

    m_commands.push_back(cmd);
    m_currentIndex++;
}

void UndoStack::undo() {
    if(m_currentIndex >= 0) {
        m_commands[m_currentIndex]->undo();
        m_currentIndex--;
    }
}

void UndoStack::redo() {
    if((m_currentIndex + 1) < m_commands.size()) {
        m_currentIndex++;
        m_commands[m_currentIndex]->redo();
    }
}

bool UndoStack::isClean() {
    return m_cleanIndex == m_currentIndex;
}

void UndoStack::setClean() {
    m_cleanIndex = m_currentIndex;
}

TString UndoStack::undoText() const {
    if(m_currentIndex >= 0) {
        return m_commands[m_currentIndex]->text();
    }
    return TString();
}

TString UndoStack::redoText() const {
    if((m_currentIndex + 1) < m_commands.size()) {
        return m_commands[m_currentIndex + 1]->text();
    }
    return TString();
}

void UndoStack::clear() {
    for(auto it : m_commands) {
        delete it;
    }
    m_commands.clear();
}
