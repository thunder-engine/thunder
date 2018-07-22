#ifndef BSON_H
#define BSON_H

#include <vector>
#include <cstdint>

#include "variant.h"

class NEXT_LIBRARY_EXPORT Bson {
public:
    static Variant              load                        (const ByteArray &data, MetaType::Type type = MetaType::VARIANTLIST);
    static ByteArray            save                        (const Variant &data);
};

#endif // BSON_H

