#ifndef ABSON
#define ABSON

#include <vector>
#include <cstdint>

#include "avariant.h"

class NEXT_LIBRARY_EXPORT ABson {
public:
    ABson                       ();

    static AVariant             load                        (const AByteArray &data, uint32_t &offset, AMetaType::Type type = AMetaType::VARIANTLIST, bool first = true);
    static AByteArray           save                        (const AVariant &data);

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
    static inline uint8_t       type                        (const AVariant &data);

};

#endif // ABSON

