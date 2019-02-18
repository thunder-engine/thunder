#include "components/angelbehaviour.h"

#include <cstring>

#include <log.h>

#include <angelscript.h>

#include <analytics/profiler.h>

AngelBehaviour::AngelBehaviour() :
        m_pObject(nullptr),
        m_pStart(nullptr),
        m_pUpdate(nullptr),
        m_pMetaObject(nullptr) {
    PROFILER_MARKER;
}

AngelBehaviour::~AngelBehaviour() {
    PROFILER_MARKER;
    if(m_pObject) {
        m_pObject->Release();
    }
}

string AngelBehaviour::script() const {
    PROFILER_MARKER;
    return m_Script;
}

void AngelBehaviour::setScript(const string &value) {
    PROFILER_MARKER;
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
    PROFILER_MARKER;
    m_pObject   = object;
    if(m_pObject) {
        asITypeInfo *info = m_pObject->GetObjectType();
        if(info) {
            if(object->GetPropertyCount() > 0) {
                void *ptr = this;
                memcpy(object->GetAddressOfProperty(0), &ptr, sizeof(void *));
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

            m_pMetaObject = new MetaObject(m_Script.c_str(),
                                           super,
                                           &AngelBehaviour::construct,
                                           nullptr,
                                           &m_Table[0]);
        }
    }
}

asIScriptFunction *AngelBehaviour::scriptStart() const {
    PROFILER_MARKER;
    return m_pStart;
}

void AngelBehaviour::setScriptStart(asIScriptFunction *function) {
    PROFILER_MARKER;
    m_pStart    = function;
}

asIScriptFunction *AngelBehaviour::scriptUpdate() const {
    PROFILER_MARKER;
    return m_pUpdate;
}

void AngelBehaviour::setScriptUpdate(asIScriptFunction *function) {
    PROFILER_MARKER;
    m_pUpdate   = function;
}

const MetaObject *AngelBehaviour::metaObject() const {
    PROFILER_MARKER;
    //if(m_pMetaObject) {
    //    return m_pMetaObject;
    //}
    return AngelBehaviour::metaClass();
}
