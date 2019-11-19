#include "undomanager.h"

UndoManager *UndoManager::m_pInstance   = nullptr;

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

void UndoManager::init() {

}
