#include "utils.h"

string Utils::wc32ToUtf8(uint32_t wc32) {
    string result;
    if(wc32 < 0x007F) {
        result  += (char)wc32;
    } else if(wc32 < 0x07FF) {
        result  += (char)(0xC0 + (wc32 >> 6));
        result  += (char)(0x80 + (wc32 & 0x3F));
    } else if(wc32 < 0xFFFF) {
        result  += (char)(0xE0 + (wc32 >> 12));
        result  += (char)(0x80 + (wc32 >> 6 & 0x3F));
        result  += (char)(0x80 + (wc32 & 0x3F));
    } else {
        result  += (char)(0xF0 + (wc32 >> 18));
        result  += (char)(0x80 + (wc32 >> 12 & 0x3F));
        result  += (char)(0x80 + (wc32 >> 6 & 0x3F));
        result  += (char)(0x80 + (wc32 & 0x3F));
    }
    return result;
}

string Utils::wstringToUtf8(const wstring &in) {
    string result;
    for(auto it : in) {
        result += wc32ToUtf8(it);
    }
    return result;
}

string Utils::utf32ToUtf8(const u32string &in) {
    string result;
    for(auto it : in) {
        result += wc32ToUtf8(it);
    }
    return result;
}

u32string Utils::utf8ToUtf32(const string &in) {
    u32string result;

    char *p         = (char *)in.c_str();
    char *lim       = p + in.size();

    uint32_t high;

    uint8_t n       = 0;
    for (; p < lim; p += n) {
        // Get number of bytes for one wide character.

        n  = 1;	// default: 1 byte. Used when skipping bytes.
        if ((*p & 0x80) == 0) {
            high    = (uint32_t)*p;
        } else if ((*p & 0xe0) == 0xc0) {
            n       = 2;
            high    = (uint32_t)(*p & 0x1f);
        } else if ((*p & 0xf0) == 0xe0) {
            n       = 3;
            high    = (uint32_t)(*p & 0x0f);
        } else if ((*p & 0xf8) == 0xf0) {
            n       = 4;
            high    = (uint32_t)(*p & 0x07);
        } else if ((*p & 0xfc) == 0xf8) {
            n       = 5;
            high    = (uint32_t)(*p & 0x03);
        } else if ((*p & 0xfe) == 0xfc) {
            n       = 6;
            high    = (uint32_t)(*p & 0x01);
        } else {
            continue;
        }
        // does the sequence header tell us truth about length?
        if(lim - p <= n - 1) {
            n = 1;
            continue;	// skip
        }
        // Validate sequence.
        // All symbols must have higher bits set to 10xxxxxx
        uint32_t i;
        if (n > 1) {
            for (i = 1; i < n; i++) {
                if ((p[i] & 0xc0) != 0x80)
                    break;
            }
            if (i != n) {
                n   = 1;
                continue;	// skip
            }
        }

        uint32_t out    = 0;
        uint32_t n_bits = 0;
        for (i = 1; i < n; i++) {
            out     |= (uint32_t)(p[n - i] & 0x3f) << n_bits;
            n_bits  += 6;
        }
        out |= high << n_bits;

        result  += out;
    }

    return result;
}
