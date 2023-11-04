/*
    This file is part of Thunder Next.

    Thunder Next is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

    Thunder Next is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Thunder Next.  If not, see <http://www.gnu.org/licenses/>.

    Copyright: 2008-2023 Evgeniy Prikazchikov
*/

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
    Constructs MetaMethod object which will contain information provided in a \a table.
*/
MetaMethod::MetaMethod(const Table *table) :
        m_table(table) {
    PROFILE_FUNCTION();
}
/*!
    Returns true if method is valid; otherwise returns false.
*/
bool MetaMethod::isValid() const {
    PROFILE_FUNCTION();
    return (m_table != nullptr);
}
/*!
    Returns a name of method.
*/
const char *MetaMethod::name() const {
    PROFILE_FUNCTION();
    return m_table->name;
}
/*!
    Returns method signature in text format.
*/
string MetaMethod::signature() const {
    PROFILE_FUNCTION();
    string sig(m_table->name);
    int pos = sig.rfind(':');
    if(pos != -1) {
        sig = sig.substr(pos + 1, sig.size() - pos);
    }
    sig += '(';

    for(int i = 0; i < m_table->argc; ++i) {
        MetaType arg(m_table->types[i + 1]);
        sig += arg.name();
        sig += ',';
    }
    if(m_table->argc > 0) {
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
    PROFILE_FUNCTION();
    return m_table->type;
}
/*!
    Returns a return type of method.
*/
MetaType MetaMethod::returnType() const {
    PROFILE_FUNCTION();
    if(m_table->types) {
        return MetaType(m_table->types[0]);
    }
    return MetaType(nullptr);
}
/*!
    Returns a parameter count of method.
*/
int MetaMethod::parameterCount() const {
    PROFILE_FUNCTION();
    return m_table->argc;
}
/*!
    Returns the type of parameter at \a index position.
*/
MetaType MetaMethod::parameterType(int index) const {
    PROFILE_FUNCTION();
    return MetaType(m_table->types[index + 1]);
}
/*!
    Calls current method for \a object.
    Function recieves an argument count in \a argc parameter and \a args array.
    Function is able to return the result of method invocation in \a returnValue.

    Return true on succssed; otherwise returns false.

    \note Function checks if current method can be invoked.

*/
bool MetaMethod::invoke(void *object, Variant &returnValue, int argc, const Variant *args) const {
    PROFILE_FUNCTION();
    if(m_table->type != Signal) {
         m_table->invoker(object, argc, args, returnValue);
        return true;
    }
    return false;
}
/*!
    Returns method information table.
*/
const MetaMethod::Table *MetaMethod::table() const {
    PROFILE_FUNCTION();
    return m_table;
}

/*!
    \class MethodCallEvent
    \brief MethodCallEvent implements event which contain all necessary information for method invocation.
    \since Next 1.0
    \inmodule Core
*/
MethodCallEvent::MethodCallEvent(int32_t method, Object *sender, const Variant &args) :
        Event(MethodCall),
        m_sender(sender),
        m_method(method),
        m_args(args) {
    PROFILE_FUNCTION();
}
/*!
    Returns the object that sent this event.
*/
Object *MethodCallEvent::sender() const {
    PROFILE_FUNCTION();
    return m_sender;
}
/*!
    Returns an index of method.
*/
int32_t MethodCallEvent::method() const {
    PROFILE_FUNCTION();
    return m_method;
}
/*!
    Returns an arguments array for method invocation.
*/
const Variant *MethodCallEvent::args() const {
    PROFILE_FUNCTION();
    return &m_args;
}
