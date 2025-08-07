#ifndef UNDOMANAGER_H
#define UNDOMANAGER_H

#include <engine.h>

class ENGINE_EXPORT UndoCommand {
public:
    explicit UndoCommand(const TString &text, UndoCommand *parent = nullptr);
    virtual ~UndoCommand();

    TString text() const;

    virtual void undo();
    virtual void redo();

    int childCount() const;

protected:
    std::vector<UndoCommand *> m_childs;

    TString m_text;

};

class ENGINE_EXPORT UndoStack {
public:
    UndoStack();
    ~UndoStack();

    void push(UndoCommand *cmd);

    void undo();
    void redo();

    bool isClean();
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
