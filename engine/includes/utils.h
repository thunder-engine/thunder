#ifndef UTILS_H
#define UTILS_H

#include "engine.h"

class ENGINE_EXPORT Utils {
public:
    static std::string wstringToUtf8(const std::wstring &in);

    static std::string utf32ToUtf8(const std::u32string &in);

    static std::u32string utf8ToUtf32(const std::string &in);

    static std::string wc32ToUtf8(uint32_t wc32);

};

#endif // UTILS_H
