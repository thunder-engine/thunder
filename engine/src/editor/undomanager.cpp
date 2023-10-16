#include "undomanager.h"

UndoManager *UndoManager::m_pInstance = nullptr;

UndoManager *UndoManager::instance() {
    if(!m_pInstance) {
        m_pInstance = new UndoManager;
    }
    return m_pInstance;
}

void UndoManager::destroy() {
    delete m_pInstance;
    m_pInstance = nullptr;
}

void UndoManager::beginGroup(QString name) {
    m_group = new QUndoCommand(name);
}

void UndoManager::endGroup() {
    if(m_group && m_group->childCount() > 0) {
        QUndoStack::push(m_group);
    } else {
        delete m_group;
    }
    m_group = nullptr;
}

QUndoCommand *UndoManager::group() {
    return m_group;
}

void UndoManager::push(UndoCommand *cmd) {
    if(m_group == nullptr) {
        QUndoStack::push(cmd);
    }
}

const UndoCommand *UndoManager::lastCommand(const QObject *editor) const {
    for(int i = index() - 1; i >= 0; i--) {
        const UndoCommand *cmd = dynamic_cast<const UndoCommand *>(command(i));
        if(cmd && cmd->editor() == editor) {
            return cmd;
        }
    }
    return nullptr;
}
