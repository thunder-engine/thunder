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

#ifndef METAENUM_H
#define METAENUM_H

#include "metatype.h"

class NEXT_LIBRARY_EXPORT MetaEnum {
public:
    struct EnumTable {
        const char *name;
        int value;
    };

    struct Table {
        const char *name;
        const EnumTable *table;
    };

public:
    explicit MetaEnum(const Table *table);

    bool isValid() const;

    const char *name() const;

    int keyCount() const;
    const char *key(int index) const;

    int value(int index) const;

    int keyToValue(const char *key) const;
    const char *valueToKey(int value) const;

    const Table *table() const;

private:
    const Table *m_table;

    int m_enumCount;

};

#endif // METAENUM_H
