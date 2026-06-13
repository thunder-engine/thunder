/*
    This file is part of Thunder Next.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "core/metaenum.h"

#include <cstring>

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
        m_table(table),
        m_enumCount(0) {
    PROFILE_FUNCTION();

    if(m_table) {
        while(m_table->table && m_table->table[m_enumCount].name) {
            m_enumCount++;
        }
    }
}
/*!
    Returns true if enumerator is valid; otherwise returns false.
*/
bool MetaEnum::isValid() const {
    PROFILE_FUNCTION();
    return (m_table != nullptr);
}
/*!
    Returns a name of enumerator.
*/
const char *MetaEnum::name() const {
    PROFILE_FUNCTION();
    return m_table->name;
}
/*!
    Returns the number of keys.
*/
int MetaEnum::keyCount() const {
    PROFILE_FUNCTION();
    return m_enumCount;
}
/*!
    Returns the key with the given \a index, or nullptr if no such key exists.
*/
const char *MetaEnum::key(int index) const {
    PROFILE_FUNCTION();
    if(index <= m_enumCount) {
        return m_table->table[index].name;
    }
    return nullptr;
}
/*!
    Returns the value with the given \a index; or returns -1 if there is no such value.
*/
int MetaEnum::value(int index) const {
    PROFILE_FUNCTION();
    if(index <= m_enumCount) {
        return m_table->table[index].value;
    }
    return -1;
}
/*!
    Returns the integer value of the given enumeration \a key, or -1 if key is not defined
*/
int MetaEnum::keyToValue(const char *key) const {
    for(int index = 0; index < m_enumCount; index++) {
        if(std::strcmp(m_table->table[index].name, key) == 0) {
            return m_table->table[index].value;
        }
    }
    return -1;
}
/*!
    Returns the string that is used as the name of the given enumeration \a value, or nullptr if value is not defined.
*/
const char *MetaEnum::valueToKey(int value) const {
    for(int index = 0; index < m_enumCount; index++) {
        if(m_table->table[index].value == value) {
            return m_table->table[index].name;
        }
    }
    return nullptr;
}
/*!
    Returns enumerator information table.
*/
const MetaEnum::Table *MetaEnum::table() const {
    PROFILE_FUNCTION();
    return m_table;
}
