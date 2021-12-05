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

const UndoCommand *UndoManager::lastCommand(const QObject *editor) const {
    for(int i = index() - 1; i >= 0; i--) {
        const UndoCommand *cmd = dynamic_cast<const UndoCommand *>(command(i));
        if(cmd && cmd->editor() == editor) {
            return cmd;
        }
    }
    return nullptr;
}
