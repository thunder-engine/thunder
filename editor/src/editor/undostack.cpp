/*
    This file is part of Thunder Engine.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
#include "undostack.h"

/*!
    \class UndoCommand
    \brief Represents an undoable editor operation.
    \inmodule Editor
*/

/*!
    \class UndoStack
    \brief Maintains the undo and redo history of an asset editor.
    \inmodule Editor
*/

/*!
    Constructs a command with the specified display \a text and optional \a parent.
*/
UndoCommand::UndoCommand(const TString &text, UndoCommand *parent) :
        m_text(text) {

    if(parent) {
        parent->m_childs.push_back(this);
    }
}

/*!
    Destroys the command and all child commands owned by it.
*/
UndoCommand::~UndoCommand() {
    for(auto it : m_childs) {
        delete it;
    }
}

/*!
    Returns the display text of the command.
*/
TString UndoCommand::text() const {
    return m_text;
}

/*!
    Reverts this command and its child commands.
*/
void UndoCommand::undo() {
    for(int i = m_childs.size() - 1; i >= 0; i--) {
        m_childs[i]->undo();
    }
}

/*!
    Applies this command and its child commands.
*/
void UndoCommand::redo() {
    for(auto it : m_childs) {
        it->redo();
    }
}

/*!
    Returns the number of child commands.
*/
size_t UndoCommand::childCount() const {
    return m_childs.size();
}

/*!
    Constructs an empty undo and redo stack.
*/
UndoStack::UndoStack() :
        m_currentIndex(-1),
        m_cleanIndex(-1) {

}

/*!
    Destroys the stack and all commands it owns.
*/
UndoStack::~UndoStack() {
    clear();
}

/*!
    Adds \a cmd to the stack and applies it.
*/
void UndoStack::push(UndoCommand *cmd) {
    if(!cmd) {
        return;
    }

    cmd->redo();

    if(m_currentIndex + 1 < m_commands.size()) {
        for(int i = m_currentIndex + 1; i < m_commands.size(); ++i) {
            delete m_commands[i];
        }
        m_commands.resize(m_currentIndex + 1);
    }

    m_commands.push_back(cmd);
    m_currentIndex++;
}

/*!
    Undoes the current command when one is available.
*/
void UndoStack::undo() {
    if(m_currentIndex >= 0) {
        m_commands[m_currentIndex]->undo();
        m_currentIndex--;
    }
}

/*!
    Redoes the next command when one is available.
*/
void UndoStack::redo() {
    if((m_currentIndex + 1) < m_commands.size()) {
        m_currentIndex++;
        m_commands[m_currentIndex]->redo();
    }
}

/*!
    Returns true if the stack is at its clean state.
*/
bool UndoStack::isClean() const {
    return m_cleanIndex == m_currentIndex;
}

/*!
    Marks the current stack position as clean.
*/
void UndoStack::setClean() {
    m_cleanIndex = m_currentIndex;
}

/*!
    Returns the text of the command that will be undone.
*/
TString UndoStack::undoText() const {
    if(m_currentIndex >= 0) {
        return m_commands[m_currentIndex]->text();
    }
    return TString();
}

/*!
    Returns the text of the command that will be redone.
*/
TString UndoStack::redoText() const {
    if((m_currentIndex + 1) < m_commands.size()) {
        return m_commands[m_currentIndex + 1]->text();
    }
    return TString();
}

/*!
    Removes all commands and resets the stack to its initial state.
*/
void UndoStack::clear() {
    for(auto it : m_commands) {
        delete it;
    }
    m_commands.clear();
    m_currentIndex = -1;
    m_cleanIndex = -1;
}
