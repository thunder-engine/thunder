#include "core/ametamethod.h"

#include "core/avariant.h"

AMetaMethod::AMetaMethod(const Table *table) :
        m_pTable(table) {
    PROFILE_FUNCTION()
}

bool AMetaMethod::isValid() const {
    PROFILE_FUNCTION()
    return (m_pTable != nullptr);
}

const char *AMetaMethod::name() const {
    PROFILE_FUNCTION()
    return m_pTable->name;
}

string AMetaMethod::signature() const {
    PROFILE_FUNCTION()
    string sig(m_pTable->name);
    int pos = sig.rfind(':');
    if(pos != -1) {
        sig = sig.substr(pos + 1, sig.size() - pos);
    }
    sig += '(';

    for(int i = 0; i < m_pTable->argc; ++i) {
        AMetaType arg(m_pTable->types[i + 1]);
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

AMetaMethod::MethodType AMetaMethod::type() const {
    PROFILE_FUNCTION()
    return m_pTable->type;
}

AMetaType AMetaMethod::returnType() const {
    PROFILE_FUNCTION()
    return AMetaType(m_pTable->types[0]);
}

int AMetaMethod::parameterCount() const {
    PROFILE_FUNCTION()
    return m_pTable->argc;
}

AMetaType AMetaMethod::parameterType(int index) const {
    PROFILE_FUNCTION()
    return AMetaType(m_pTable->types[index + 1]);
}

bool AMetaMethod::invoke(AObject *obj, AVariant &returnValue, int argc, const AVariant *args) const {
    PROFILE_FUNCTION()
    if(m_pTable->type != Signal) {
        returnValue = m_pTable->invoker(obj, argc, args);
        return true;
    }
    return false;
}

AMethodCallEvent::AMethodCallEvent(int32_t method, AObject *sender, const AVariant &args) :
        AEvent(MethodCall),
        m_pSender(sender),
        m_Method(method),
        m_Args(args) {
    PROFILE_FUNCTION()
}

AObject *AMethodCallEvent::sender() const {
    PROFILE_FUNCTION()
    return m_pSender;
}

int32_t AMethodCallEvent::method() const {
    PROFILE_FUNCTION()
    return m_Method;
}

const AVariant *AMethodCallEvent::args() const {
    PROFILE_FUNCTION()
    return &m_Args;
}
