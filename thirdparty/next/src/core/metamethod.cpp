#include "core/metamethod.h"

#include "core/variant.h"
/*!
    \class MetaMethod
    \brief The MetaMethod provides an interface to retrieve information about object method at runtime.
    \since Next 1.0
    \inmodule Core

    This class is a part of Object-Introspection-Mechanism. MetaMethod provides information about one particular class method.
    Developers are able to retrieve information about method arguments, return types and etc.

    To make methods visible in introspection mechanism, developers must declare those under A_METHODS() macro.
*/
/*!
    \enum MetaMethod::MethodType

    This enum defines base method types.

    \value Method \c Standard method can be invoked. Used for general porposes.
    \value Signal \c Method without impelementation can't be invoked. Used for Signals and Slots mechanism.
    \value Slot \c Very similar to A_METHOD but with special flag to be used for Signals and Slots mechanism.
*/
/*!
    \typedef MetaMethod::Table::InvokeMem

    Callback which contain address to method for invocation.
*/
/*!
    Constructs MetaMethod object wich will contain information provided in a \a table.
*/
MetaMethod::MetaMethod(const Table *table) :
        m_pTable(table) {
    PROFILE_FUNCTION()
}
/*!
    Returns true if property is valid; otherwise returns false.
*/
bool MetaMethod::isValid() const {
    PROFILE_FUNCTION()
    return (m_pTable != nullptr);
}
/*!
    Returns a name of method.
*/
const char *MetaMethod::name() const {
    PROFILE_FUNCTION()
    return m_pTable->name;
}
/*!
    Returns method signature in text format.
*/
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
/*!
    Returns a type of method.
*/
MetaMethod::MethodType MetaMethod::type() const {
    PROFILE_FUNCTION()
    return m_pTable->type;
}
/*!
    Returns a return type of method.
*/
MetaType MetaMethod::returnType() const {
    PROFILE_FUNCTION()
    return MetaType(m_pTable->types[0]);
}
/*!
    Returns a parameter count of method.
*/
int MetaMethod::parameterCount() const {
    PROFILE_FUNCTION()
    return m_pTable->argc;
}
/*!
    Returns the type of parameter at \a index position.
*/
MetaType MetaMethod::parameterType(int index) const {
    PROFILE_FUNCTION()
    return MetaType(m_pTable->types[index + 1]);
}
/*!
    Calls current method for \a object.
    Function recieves an argument count in \a argc parameter and \a args array.
    Function is able to return the result of method invocation in \a returnValue.

    Return true on succssed; otherwise returns false.

    \note Function checks if current method can be invoked.

*/
bool MetaMethod::invoke(Object *object, Variant &returnValue, int argc, const Variant *args) const {
    PROFILE_FUNCTION()
    if(m_pTable->type != Signal) {
        returnValue = m_pTable->invoker(object, argc, args);
        return true;
    }
    return false;
}
/*!
    \class MethodCallEvent
    \brief MethodCallEvent implements event which contain all necessary information for method invocation.
    \since Next 1.0
    \inmodule Core
*/
MethodCallEvent::MethodCallEvent(int32_t method, Object *sender, const Variant &args) :
        Event(METHODCALL),
        m_pSender(sender),
        m_Method(method),
        m_Args(args) {
    PROFILE_FUNCTION()
}
/*!
    Returns the object that sent this event.
*/
Object *MethodCallEvent::sender() const {
    PROFILE_FUNCTION()
    return m_pSender;
}
/*!
    Returns an index of method.
*/
int32_t MethodCallEvent::method() const {
    PROFILE_FUNCTION()
    return m_Method;
}
/*!
    Returns an arguments array for method invocation.
*/
const Variant *MethodCallEvent::args() const {
    PROFILE_FUNCTION()
    return &m_Args;
}
