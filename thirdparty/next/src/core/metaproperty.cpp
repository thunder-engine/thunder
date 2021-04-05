#include "core/metaproperty.h"

#include <cstring>

/*!
    \class MetaProperty
    \brief The MetaProperty provides an interface to retrieve information about object property at runtime.
    \since Next 1.0
    \inmodule Core

    This class is a part of Object-Introspection-Mechanism. MetaProperty provides information about one particular class property.
    Developers are able to retrieve information about property type, read and write values.

    To make properties visible in introspection mechanism, developers must declare those under A_PROPERTIES() macro.
*/
/*!
    \typedef MetaProperty::ReadMem

    Callback which contain address to getter method of property.
*/
/*!
    \typedef MetaProperty::WriteMem

    Callback which contain address to setter method of property.
*/
/*!
    \fn void MetaProperty::write(Object *object, const T &value) const

    Tries to write a \a value with type T to provided \a object.
*/
/*!
    Constructs MetaProperty object which will contain information provided in a \a table.
*/
MetaProperty::MetaProperty(const Table *table) :
        m_pTable(table) {
    PROFILE_FUNCTION();
}
/*!
    Returns a name of method.
*/
const char *MetaProperty::name() const {
    PROFILE_FUNCTION();
    return m_pTable->name;
}
/*!
    Returns true if property is valid; otherwise returns false.
*/
bool MetaProperty::isValid() const {
    PROFILE_FUNCTION();
    return (m_pTable != nullptr);
}
/*!
    Returns a type of property.
*/
const MetaType MetaProperty::type() const {
    PROFILE_FUNCTION();
    return MetaType(m_pTable->type);
}
/*!
    Returns the value as Variant which contain current property of provided \a object.
*/
Variant MetaProperty::read(const void *object) const {
    PROFILE_FUNCTION();
    if(m_pTable->reader) {
        return m_pTable->reader(object);
    } else if(m_pTable->readproperty) {
        return m_pTable->readproperty(object, *this);
    }
    return Variant();
}
/*!
    Tries to write a \a value as Variant to provided \a object.
*/
void MetaProperty::write(void *object, const Variant &value) const {
    PROFILE_FUNCTION();
    if(m_pTable->writer) {
        m_pTable->writer(object, value);
    } else if(m_pTable->writeproperty) {
        m_pTable->writeproperty(object, *this, value);
    }
}
/*!
    Returns property information table.
*/
const MetaProperty::Table *MetaProperty::table() const {
    PROFILE_FUNCTION();
    return m_pTable;
}
