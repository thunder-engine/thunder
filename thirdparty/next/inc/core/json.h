#ifndef JSON_H
#define JSON_H

#include <string>
#include <cstdint>

#include "variant.h"

class NEXT_LIBRARY_EXPORT Json {
public:
    static Variant              load                        (const string &data);
    static string               save                        (const Variant &data, int32_t tab = -1);
};

#endif // JSON_H
