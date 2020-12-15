#include "core/metaenum.h"

/*!
    \class MetaEnum
    \brief The MetaEnum provides an interface to retrieve information about object enumerator at runtime.
    \since Next 1.0
    \inmodule Core

    This class is a part of Object-Introspection-Mechanism. MetaEnum provides information about one particular class enumerator.

    To make enumerators visible in introspection mechanism, developers must declare those under A_ENUMS() macro.
*/

/*!
    Constructs MetaEnum object which will contain information provided in a \a table.
*/
MetaEnum::MetaEnum(const Table *table) :
        m_pTable(table),
        m_EnumCount(0) {
    PROFILE_FUNCTION();

    while(table->table && table->table[m_EnumCount].name) {
        m_EnumCount++;
    }
}
/*!
    Returns true if enumerator is valid; otherwise returns false.
*/
bool MetaEnum::isValid() const {
    PROFILE_FUNCTION();
    return (m_pTable != nullptr);
}
/*!
    Returns a name of enumerator.
*/
const char *MetaEnum::name() const {
    PROFILE_FUNCTION();
    return m_pTable->name;
}
/*!
    Returns the number of keys.
*/
int MetaEnum::keyCount() const {
    PROFILE_FUNCTION();
    return m_EnumCount;
}
/*!
    Returns the key with the given \a index, or nullptr if no such key exists.
*/
const char *MetaEnum::key(int index) const {
    PROFILE_FUNCTION();
    if(index <= m_EnumCount) {
        return m_pTable->table[index].name;
    }
    return nullptr;
}
/*!
    Returns the value with the given \a index; or returns -1 if there is no such value.
*/
int MetaEnum::value(int index) const {
    PROFILE_FUNCTION();
    if(index <= m_EnumCount) {
        return m_pTable->table[index].value;
    }
    return -1;
}
/*!
    Returns enumerator information table.
*/
const MetaEnum::Table *MetaEnum::table() const {
    PROFILE_FUNCTION();
    return m_pTable;
}
