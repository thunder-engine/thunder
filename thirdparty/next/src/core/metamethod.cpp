#include "core/metamethod.h"

#include "core/variant.h"

MetaMethod::MetaMethod(const Table *table) :
        m_pTable(table) {
    PROFILE_FUNCTION()
}

bool MetaMethod::isValid() const {
    PROFILE_FUNCTION()
    return (m_pTable != nullptr);
}

const char *MetaMethod::name() const {
    PROFILE_FUNCTION()
    return m_pTable->name;
}

string MetaMethod::signature() const {
    PROFILE_FUNCTION()
    string sig(m_pTable->name);
    int pos = sig.rfind(':');
    if(pos != -1) {
        sig = sig.substr(pos + 1, sig.size() - pos);
    }
    sig += '(';

    for(int i = 0; i < m_pTable->argc; ++i) {
        MetaType arg(m_pTable->types[i + 1]);
        sig += arg.name();
        sig += ',';
    }
    if(m_pTable->argc > 0) {
        sig[sig.size() - 1] = ')';
    } else {
        sig += ')';
    }

    return sig;
}

MetaMethod::MethodType MetaMethod::type() const {
    PROFILE_FUNCTION()
    return m_pTable->type;
}

MetaType MetaMethod::returnType() const {
    PROFILE_FUNCTION()
    return MetaType(m_pTable->types[0]);
}

int MetaMethod::parameterCount() const {
    PROFILE_FUNCTION()
    return m_pTable->argc;
}

MetaType MetaMethod::parameterType(int index) const {
    PROFILE_FUNCTION()
    return MetaType(m_pTable->types[index + 1]);
}

bool MetaMethod::invoke(Object *obj, Variant &returnValue, int argc, const Variant *args) const {
    PROFILE_FUNCTION()
    if(m_pTable->type != Signal) {
        returnValue = m_pTable->invoker(obj, argc, args);
        return true;
    }
    return false;
}

MethodCallEvent::MethodCallEvent(int32_t method, Object *sender, const Variant &args) :
        Event(MethodCall),
        m_pSender(sender),
        m_Method(method),
        m_Args(args) {
    PROFILE_FUNCTION()
}

Object *MethodCallEvent::sender() const {
    PROFILE_FUNCTION()
    return m_pSender;
}

int32_t MethodCallEvent::method() const {
    PROFILE_FUNCTION()
    return m_Method;
}

const Variant *MethodCallEvent::args() const {
    PROFILE_FUNCTION()
    return &m_Args;
}
