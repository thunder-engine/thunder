#include "core/metaobject.h"

#include "core/object.h"

#include <cstring>

MetaObject::MetaObject(const char *name, const MetaObject *super, const Constructor constructor, const MetaMethod::Table *methods, const MetaProperty::Table *props) :
        m_pName(name),
        m_Constructor(constructor),
        m_pSuper(super),
        m_pMethods(methods),
        m_pProperties(props),
        m_MethodCount(0),
        m_PropCount(0) {
    PROFILE_FUNCTION()
    while(methods && methods[m_MethodCount].name) {
        m_MethodCount++;
    }
    while(props && props[m_PropCount].type) {
        m_PropCount++;
    }
}

const char *MetaObject::name() const {
    PROFILE_FUNCTION()
    return m_pName;
}

const MetaObject *MetaObject::super() const {
    PROFILE_FUNCTION()
    return m_pSuper;
}

Object *MetaObject::createInstance() const {
    PROFILE_FUNCTION()
    return (*m_Constructor)();
}

int MetaObject::indexOfMethod(const char *signature) const {
    PROFILE_FUNCTION()
    const MetaObject *s    = this;

    while(s) {
        for(int i = 0; i < s->m_MethodCount; ++i) {
            if(MetaMethod(m_pMethods + i).signature() == signature) {
                return i + s->methodOffset();
            }
        }
        s = s->m_pSuper;
    }
    return -1;
}

int MetaObject::indexOfSignal(const char *signature) const {
    PROFILE_FUNCTION()
    const MetaObject *s    = this;

    while(s) {
        for(int i = 0; i < s->m_MethodCount; ++i) {
            MetaMethod m(m_pMethods + i);
            if(m.type() == MetaMethod::Signal && m.signature() == signature) {
                return i + s->methodOffset();
            }
        }
        s = s->m_pSuper;
    }
    return -1;
}

int MetaObject::indexOfSlot(const char *signature) const {
    PROFILE_FUNCTION()
    const MetaObject *s    = this;

    while(s) {
        for(int i = 0; i < s->m_MethodCount; ++i) {
            MetaMethod m(m_pMethods + i);
            if(m.type() == MetaMethod::Slot && m.signature() == signature) {
                return i + s->methodOffset();
            }
        }
        s = s->m_pSuper;
    }
    return -1;
}

MetaMethod MetaObject::method(int index) const {
    PROFILE_FUNCTION()
    int i = index - methodOffset();
    if(i < 0 && m_pSuper) {
        return m_pSuper->method(index);
    }

    if(i >= 0 && i < m_MethodCount) {
        return MetaMethod(m_pMethods + index);
    }
    return MetaMethod(nullptr);
}

int MetaObject::methodCount() const {
    PROFILE_FUNCTION()
    int count               = m_MethodCount;
    const MetaObject *s    = m_pSuper;
    while(s) {
        count  += s->m_MethodCount;
        s       = s->m_pSuper;
    }
    return count;
}

int MetaObject::methodOffset() const {
    PROFILE_FUNCTION()
    int offset              = 0;
    const MetaObject *s    = m_pSuper;
    while(s) {
        offset += s->m_MethodCount;
        s       = s->m_pSuper;
    }
    return offset;
}

int MetaObject::indexOfProperty(const char *name) const {
    PROFILE_FUNCTION()
    const MetaObject *s    = this;

    while(s) {
        for(int i = 0; i < s->m_PropCount; ++i) {
            if(strcmp(s->m_pProperties[i].name, name) == 0) {
                return i + s->propertyOffset();
            }
        }
        s   = s->m_pSuper;
    }
    return -1;
}

MetaProperty MetaObject::property(int index) const {
    PROFILE_FUNCTION()
    int i = index - propertyOffset();
    if(i < 0 && m_pSuper) {
        return m_pSuper->property(index);
    }
    if(i >= 0 && i < m_PropCount) {
        return MetaProperty(m_pProperties + index);
    }
    return MetaProperty(nullptr);
}

int MetaObject::propertyCount() const {
    PROFILE_FUNCTION()
    int count               = m_PropCount;
    const MetaObject *s    = m_pSuper;
    while(s) {
        count  += s->m_PropCount;
        s       = s->m_pSuper;
    }
    return count;
}

int MetaObject::propertyOffset() const {
    PROFILE_FUNCTION()
    int offset              = 0;
    const MetaObject *s    = m_pSuper;
    while(s) {
        offset += s->m_PropCount;
        s       = s->m_pSuper;
    }
    return offset;
}

bool MetaObject::canCastTo(const char *type) const {
    PROFILE_FUNCTION()
    const MetaObject *s    = this;

    while(s) {
        if(strcmp(s->m_pName, type) == 0) {
            return true;
        }
        s   = s->m_pSuper;
    }
    return false;
}
