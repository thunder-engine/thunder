#ifndef AJSON_H
#define AJSON_H

#include <string>
#include <list>
#include <map>
#include <stack>
#include <cstdint>

#include "avariant.h"

class NEXT_LIBRARY_EXPORT AJson {
public:
    AJson                       ();

    static AVariant             load                        (const string &data);
    static string               save                        (const AVariant &data, int32_t depth = -1);

    static inline string        readString                  (const string &data, uint32_t &it);

protected:
    static inline void          skipSpaces                  (const char *data, uint32_t &it);

    static inline bool          isSpace                     (uint8_t c);
    static inline bool          isDigit                     (uint8_t c);

};

#endif // AJSON_H
