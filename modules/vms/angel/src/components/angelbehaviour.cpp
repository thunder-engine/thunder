#include "components/angelbehaviour.h"

#include <log.h>

#include <angelscript.h>

AngelBehaviour::AngelBehaviour() :
        m_pObject(nullptr),
        m_pStart(nullptr),
        m_pUpdate(nullptr) {

}

AngelBehaviour::~AngelBehaviour() {
    if(m_pObject) {
        m_pObject->Release();
    }
}

string AngelBehaviour::script() const {
    return m_Script;
}

void AngelBehaviour::setScript(const string &value) {
    if(value != m_Script) {
        if(m_pObject) {
            m_pObject->Release();
            m_pObject   = 0;
        }
        m_Script    = value;
    }
}

asIScriptObject *AngelBehaviour::scriptObject() const {
    return m_pObject;
}

void AngelBehaviour::setScriptObject(asIScriptObject *object) {
    m_pObject   = object;
}

asIScriptFunction *AngelBehaviour::scriptStart() const {
    return m_pStart;
}

void AngelBehaviour::setScriptStart(asIScriptFunction *function) {
    m_pStart    = function;
}

asIScriptFunction *AngelBehaviour::scriptUpdate() const {
    return m_pUpdate;
}

void AngelBehaviour::setScriptUpdate(asIScriptFunction *function) {
    m_pUpdate   = function;
}
