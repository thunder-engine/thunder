#ifndef BSON_H
#define BSON_H

#include <vector>
#include <cstdint>

#include "variant.h"

class NEXT_LIBRARY_EXPORT Bson {
public:
    Bson                        ();

    static Variant              load                        (const ByteArray &data, uint32_t &offset, MetaType::Type type = MetaType::VARIANTLIST, bool first = true);
    static ByteArray            save                        (const Variant &data);

protected:
    enum BsonDataTypes {
        DOUBLE                  = 1,
        STRING,
        OBJECT,
        ARRAY,
        BINARY,
        UNDEFINED,
        OBJECTID,
        BOOL,
        DATETYME,
        NONE,
        REGEXP,
        DBPOINTER,
        JAVASCRIPT,
        DEPRECATED,
        JAVASCRIPTWS,
        INT32,
        TIMESTAMP,
        INT64,
        MINKEY                  = -1,
        MAXKEY                  = 127
    };

protected:
    static inline uint8_t       type                        (const Variant &data);

};

#endif // BSON_H

