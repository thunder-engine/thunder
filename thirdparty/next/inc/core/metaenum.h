#ifndef METAENUM_H
#define METAENUM_H

#include "metatype.h"

class NEXT_LIBRARY_EXPORT MetaEnum {
public:
    struct EnumTable {
        const char     *name;
        int             value;
    };

    struct Table {
        const char      *name;
        const EnumTable *table;
    };

public:
    explicit MetaEnum   (const Table *table);

    bool                isValid         () const;

    const char         *name            () const;

    int                 keyCount        () const;
    const char         *key             (int index) const;

    int                 value           (int index) const;

    const Table        *table          () const;

private:
    const Table        *m_pTable;

    int                 m_EnumCount;

};

#endif // METAENUM_H
