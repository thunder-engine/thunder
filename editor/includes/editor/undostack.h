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
#ifndef UNDOMANAGER_H
#define UNDOMANAGER_H

#include <editor.h>

class EDITOR_EXPORT UndoCommand {
public:
    explicit UndoCommand(const TString &text, UndoCommand *parent = nullptr);
    virtual ~UndoCommand();

    TString text() const;

    virtual void undo();
    virtual void redo();

    size_t childCount() const;

protected:
    std::vector<UndoCommand *> m_childs;

    TString m_text;

};

class EDITOR_EXPORT UndoStack {
public:
    UndoStack();
    ~UndoStack();

    void push(UndoCommand *cmd);

    void undo();
    void redo();

    bool isClean() const;
    void setClean();

    TString undoText() const;
    TString redoText() const;

    void clear();

protected:
    std::vector<UndoCommand *> m_commands;

    int m_currentIndex;

    int m_cleanIndex;

};

#endif // UNDOMANAGER_H
