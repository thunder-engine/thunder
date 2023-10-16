#ifndef UNDOMANAGER_H
#define UNDOMANAGER_H

#include <QUndoCommand>

#include <engine.h>

class ENGINE_EXPORT UndoCommand : public QUndoCommand {
public:
    explicit UndoCommand(const QString &text, QObject *editor = nullptr, QUndoCommand *parent = nullptr) :
        QUndoCommand(text, parent),
        m_editor(editor) {

    }

    QObject *editor() const {
        return m_editor;
    }

private:
    QObject *m_editor;
};

class ENGINE_EXPORT UndoManager : public QUndoStack {
    Q_OBJECT

public:
    static UndoManager *instance();

    static void destroy();

    void beginGroup(QString name = QString());
    void endGroup();

    QUndoCommand *group();

    void push(UndoCommand *cmd);

    const UndoCommand *lastCommand(const QObject *editor) const;

private:
    UndoManager() {}
    ~UndoManager() {}

    static UndoManager *m_pInstance;

    QUndoCommand *m_group = nullptr;

};

#endif // UNDOMANAGER_H
