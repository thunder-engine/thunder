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
        BOOL                    = 8,
        DATETYME,
        NONE                    = 10,
        INT32                   = 16,
        MINKEY                  = -1,
        MAXKEY                  = 127,
        VECTOR2,
        VECTOR3,
        VECTOR4,
        MATRIX3,
        MATRIX4,
        QUATERNION
    };

protected:
    static inline uint8_t       type                        (const Variant &data);

};

#endif // BSON_H

