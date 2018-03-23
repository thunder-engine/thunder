#ifndef JSON_H
#define JSON_H

#include <string>
#include <list>
#include <map>
#include <stack>
#include <cstdint>

#include "variant.h"

class NEXT_LIBRARY_EXPORT Json {
public:
    Json                        ();

    static Variant              load                        (const string &data);
    static string               save                        (const Variant &data, int32_t depth = -1);

    static inline string        readString                  (const string &data, uint32_t &it);

protected:
    static inline void          skipSpaces                  (const char *data, uint32_t &it);

    static inline bool          isSpace                     (uint8_t c);
    static inline bool          isDigit                     (uint8_t c);

};

#endif // JSON_H
