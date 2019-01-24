#include "components/angelbehaviour.h"

#include <cstring>

#include <log.h>

#include <angelscript.h>

AngelBehaviour::AngelBehaviour() :
        m_pObject(nullptr),
        m_pStart(nullptr),
        m_pUpdate(nullptr),
        m_pMetaObject(nullptr) {

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
    if(m_pObject) {
        asITypeInfo *info = m_pObject->GetObjectType();
        if(info) {
            if(object->GetPropertyCount() > 0) {
                memcpy(object->GetAddressOfProperty(0), this, sizeof(void *));
            }

            if(m_pMetaObject) {
                delete m_pMetaObject;
            }
            const MetaObject *super = AngelBehaviour::metaClass();

            uint32_t count = info->GetPropertyCount();
            for(uint32_t i = 0; i <= count; i++) {
                if(i == count) {
                    m_Table.push_back({nullptr, nullptr, nullptr, nullptr, nullptr, nullptr});
                } else {
                    const char *name;
                    int typeId;
                    bool isPrivate;
                    bool isProtected;
                    int offset;
                    info->GetProperty(i, &name, &typeId, &isPrivate, &isProtected, &offset);
                    if(!isPrivate && !isProtected) {
                        m_Table.push_back({name, nullptr, nullptr, nullptr, nullptr, nullptr});
                    }
                }
            }

            m_pMetaObject = new MetaObject(m_Script.c_str(), super, &AngelBehaviour::construct, nullptr, &m_Table[0]);
        }
    }
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

const MetaObject *AngelBehaviour::metaObject() const {
    if(m_pMetaObject) {
        return m_pMetaObject;
    }
    return AngelBehaviour::metaClass();
}
