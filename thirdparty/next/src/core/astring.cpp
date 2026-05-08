#include "astring.h"

#include <algorithm>
#include <iostream>
#include <sstream>

/*!
    \class TString
    \brief The TString class provides a Unicode character string.
    \since Next 1.0
    \inmodule Core
*/

/*!
    Constructs an empty string.
*/
TString::TString() {

}
/*!
    Constructs a string from a byte array.
*/
TString::TString(const ByteArray &array) {
    m_data = std::string(array.begin(), array.end());
}
/*!
    Constructs a string from a standard string.
*/
TString::TString(const std::string &str) :
        m_data(str) {

}
/*!
    Constructs a string from a C-style null-terminated string.
*/
TString::TString(const char *str) :
        m_data(str) {

}
/*!
    Constructs a string of the given \a n size with every character set to \a ch.
*/
TString::TString(int n, const char ch) {
    m_data.resize(n);
    std::fill(m_data.begin(), m_data.end(), ch);
}
/*!
    Returns true if this string is equal to \a other; otherwise returns false.

    The comparison is case-sensitive.
*/
bool TString::operator== (const TString &other) const {
    return m_data == other.m_data;
}
/*!
    Returns true if this string is NOT equal to \a other; otherwise returns false.

    The comparison is case-sensitive.
*/
bool TString::operator!= (const TString &other) const {
    return m_data != other.m_data;
}
/*!
    Returns true if this is lexically less than \a other; otherwise returns false.
*/
bool TString::operator< (const TString &other) const {
    return m_data < other.m_data;
}
/*!
    Returns the character at the specified \a position in the string as a modifiable reference.
*/
char &TString::operator[] (int position) {
    return m_data[position];
}
/*!
    Returns a string that is the result of concatenating this string and \a other.
*/
TString TString::operator+ (const TString &other) const {
    return m_data + other.m_data;
}
/*!
    Returns a string that is the result of concatenating this string and \a other standard string.
*/
TString TString::operator+ (const std::string &other) const {
    return m_data + other;
}
/*!
    Returns a string that is the result of concatenating this string and null terminated \a other charcter string.
*/
TString TString::operator+ (const char *other) const {
    return m_data + other;
}
/*!
    Returns a string that is the result of concatenating this string and \a ch character.
*/
TString TString::operator+ (char ch) const {
    return m_data + ch;
}
/*!
    Appends the string \a other onto the end of this string and returns a reference to this string.
*/
TString &TString::operator+= (const TString &other) {
    m_data += other.m_data;
    return *this;
}
/*!
    Appends the standard string \a str to this string.
*/
TString &TString::operator+= (const std::string &str) {
    m_data += str;
    return *this;
}
/*!
    Appends the string \a str to this string. The const char pointer is converted to Unicode using the fromUtf8() function.
*/
TString &TString::operator+= (const char *str) {
    m_data += str;
    return *this;
}
/*!
    Appends the character \a ch to this string.
*/
TString &TString::operator+= (const char ch) {
    m_data += ch;
    return *this;
}
/*!
    Appends the string \a str onto the end of this string.
*/
TString &TString::append(const TString &str) {
    m_data.append(str.data());
    return *this;
}
/*!
    Appends the standard string \a str onto the end of this string.
*/
TString &TString::append(const std::string &str) {
    m_data.append(str);
    return *this;
}
/*!
    Appends the string \a str to this string. The given const char pointer is converted to Unicode.
*/
TString &TString::append(const char *str) {
    m_data.append(str);
    return *this;
}
/*!
    Appends a string of the given \a n size with every character set to \a ch.
*/
TString &TString::append(const char ch, int n) {
    m_data.append(n, ch);
    return *this;
}
/*!
    Returns the character at the given index \a position in the string.
*/
char TString::at(int position) const {
    return m_data.at(position);
}
/*!
    Returns the last character in the string.
*/
char TString::back() const {
    return m_data.back();
}
/*!
    Clears the contents of the string and makes it empty.
*/
void TString::clear() {
    m_data.clear();
}
/*!
    Compares this string with \a other string and returns a negative integer if this is less than \a other, a positive integer if it is greater than \a other, and zero if they are equal.
*/
int TString::compare(const TString &other) const {
    return m_data.compare(other.m_data);
}
/*!
    Returns true if this string contains an occurrence of the string \a str; otherwise returns false.
*/
bool TString::contains(const TString &str) const {
    return m_data.find(str.data()) != std::string::npos;
}
/*!
    Returns a pointer to the data stored in the TString.
*/
const char *TString::data() const {
    return m_data.c_str();
}
/*!
    Joins all the string \a list strings into a single string with each element separated by the given \a separator (which can be an empty string).
*/
TString TString::join(const StringList &list, const char *separator) {
    TString s;

    int i = 0;
    for(auto &it : list) {
        s += it;
        if(i >= (list.size() - 1)) {
            break; // escaping in the last iteration
        }
        i++;
        s += separator; // concatenating string
    }
    return s;
}
/*!
    Returns true if string is empty; otherwise returns false.
*/
bool TString::isEmpty() const {
    return m_data.empty();
}
/*!
    Returns true if this string starts with the string \a str; otherwise returns false.
    The comparison is case-sensitive.
*/
bool TString::startsWith(const TString &str) const {
    if (str.length() > length()) {
        return false;
    }
    return m_data.compare(0, str.length(), str.m_data) == 0;
}
/*!
    Returns the index position of the first occurrence of the string \a str in this string. Returns -1 if \a str is not found.
*/
int TString::indexOf(const TString &str) const {
     return m_data.find(str.m_data);
}
/*!
    Returns the index position of the first occurrence of the character \a ch in this string. Returns -1 if \a ch is not found.
*/
int TString::indexOf(const char ch) const {
    return m_data.find(ch);
}
/*!
    Returns a string that contains \a n characters of this string, starting at the specified \a position index up to, but not including.
*/
TString TString::mid(int position, int n) const {
    return m_data.substr(position, n);
}
/*!
    Returns the index position of the last occurrence of the string \a str in this string, searching backward from index position from.
*/
int TString::lastIndexOf(const TString &str) const {
    return m_data.rfind(str.m_data);
}
/*!
    Returns the index position of the last occurrence of the character \a ch in this string, searching backward from index position from.
*/
int TString::lastIndexOf(const char ch) const {
    return m_data.rfind(ch);
}
/*!
    Returns the number of characters in this string.
*/
int TString::length() const {
    return m_data.length();
}
/*!
    Returns a substring that contains the \a n leftmost characters of this string.
*/
TString TString::left(int n) const {
    return m_data.substr(0, n);
}
/*!
    Returns a string representing the long integer number \a in.
*/
TString TString::number(long long in) {
    std::stringstream stream;
    stream << in;
    return stream.str();
}
/*!
    Returns a string representing the integer number \a in.
*/
TString TString::number(int in) {
    std::stringstream stream;
    stream << in;
    return stream.str();
}
/*!
    Returns a string representing the floating-point number \a in.
*/
TString TString::number(float in) {
    return std::to_string(in);
}
/*!
    Removes every occurrence of the given \a str string in this string, and returns a reference to this string.
*/
TString &TString::remove(const TString &str) {
    std::string::size_type i = m_data.find(str.toStdString());

    if(i != std::string::npos) {
        m_data.erase(i, str.length());
    }

    return *this;
}
/*!
    Removes every occurrence of the character \a ch in this string, and returns a reference to this string.
*/
TString &TString::remove(const char ch) {
    m_data.erase(std::remove(m_data.begin(), m_data.end(), ch));
    return *this;
}
/*!
    Replaces every occurrence of the string \a before with the string \a after and returns a reference to this string.
*/
TString &TString::replace(const TString &before, const TString &after) {
    std::size_t replaceStrLen = after.length();
    for(std::size_t pos = 0; pos != std::string::npos; pos += replaceStrLen) {
        if((pos = m_data.find(before.m_data, pos)) != std::string::npos) {
            m_data.replace(pos, before.length(), after.m_data);
        } else {
            break;
        }
    }
    return *this;
}
/*!
    Replaces every occurrence of the character \a before with the character \a after and returns a reference to this string.
*/
TString &TString::replace(const char before, const char after) {
    std::replace(m_data.begin(), m_data.end(), before, after);
    return *this;
}
/*!
    Removes the first character in this string. If the string is empty, this function does nothing.
*/
TString &TString::removeFirst() {
    m_data.erase(m_data.begin());
    return *this;
}
/*!
    Removes the last character in this string. If the string is empty, this function does nothing.
*/
TString &TString::removeLast() {
    m_data.pop_back();
    return *this;
}
/*!
    Returns a substring that contains the \a n rightmost characters of the string.
*/
TString TString::right(int n) const {
    return m_data.substr(n, m_data.size() - n);
}
/*!
    Returns the number of characters in this string.
*/
int TString::size() const {
    return m_data.size();
}
/*!
    Splits the string into substrings wherever \a sep occurs, and returns the list of those strings.
*/
StringList TString::split(const char sep) const {
    std::istringstream stream(m_data);
    std::istringstream &f = stream;

    StringList result;

    std::string str;
    while(std::getline(f, str, sep)) {
        result.push_back(str);
    }
    return result;
}
/*!
    Splits the string into substrings wherever \a sep occurs, and returns the list of those strings.
*/
StringList TString::split(const TString &sep) const {
    StringList result;

    size_t posEnd;
    size_t posStart = 0;
    size_t delimLen = sep.length();

    while ((posEnd = m_data.find(sep.toStdString(), posStart)) != std::string::npos) {
        result.push_back(m_data.substr(posStart, posEnd - posStart));
        posStart = posEnd + delimLen;
    }

    result.push_back(m_data.substr(posStart));

    return result;
}
/*!
    Returns a std::string object with the data contained in this TString.
*/
const std::string &TString::toStdString() const {
    return m_data;
}
/*!
    Returns a std::wstring object with the data contained in this TString.
*/
std::wstring TString::toStdWString() const {
    return std::wstring(m_data.begin(), m_data.end());
}
/*!
    Returns a lowercase copy of the string.
*/
TString TString::toLower() const {
    std::string ret(m_data);

    std::transform(ret.begin(), ret.end(), ret.begin(), ::tolower);

    return TString(ret);
}
/*!
    Returns an uppercase copy of the string.
*/
TString TString::toUpper() const {
    std::string ret(m_data);

    std::transform(ret.begin(), ret.end(), ret.begin(), ::toupper);

    return TString(ret);
}
/*!
    Returns the string converted to a float value.
*/
float TString::toFloat() const {
    return std::stof(m_data);
}
/*!
    Returns the string converted to an int. Returns 0 if the conversion fails.
*/
int TString::toInt() const {
    return std::stoi(m_data);
}
/*!
    Returns the string converted to a long. Returns 0 if the conversion fails.
*/
size_t TString::toLong() const {
    return std::stoull(m_data);
}

// trim from end of string (right)
std::string &rtrim(std::string &s, const char *t) {
    s.erase(s.find_last_not_of(t) + 1);
    return s;
}

// trim from beginning of string (left)
std::string &ltrim(std::string &s, const char *t) {
    s.erase(0, s.find_first_not_of(t));
    return s;
}
/*!
    Returns a string that has whitespace removed from the start and the end.
    This includes the ASCII characters '\\t', '\\n', '\\v', '\\f', '\\r', and ' '.
*/
TString TString::trimmed() const {
    return trim(" \t\n\r\f\v");
}

TString TString::trim(const char *t) const {
    std::string ret = m_data;

    if(m_data.empty()) {
        return *this;
    }
    return rtrim(ltrim(ret, t), t);
}

static inline bool isWhitespace(char ch) {
    return ch == ' ' || ch == '\t' || ch == '\n' ||
           ch == '\r' || ch == '\v' || ch == '\f';
}
/*!
    Returns a string that has whitespace removed from the start and the end, and that has each sequence of internal whitespace replaced with a single space.
*/
TString TString::simplified() const {
    if(m_data.empty()) {
        return TString();
    }

    std::string result;
    bool lastWasSpace = true;

    for(char ch : m_data) {
        if(isWhitespace(ch)) {
            if(!lastWasSpace) {
                result.push_back(' ');
                lastWasSpace = true;
            }
        } else {
            result.push_back(ch);
            lastWasSpace = false;
        }
    }

    if(!result.empty() && result.back() == ' ') {
        result.pop_back();
    }

    return TString(result);

    return result;
}
/*!
    Returns a reference to the first character in the string.
*/
char TString::front() const {
    return m_data.front();
}
/*!
    Returns a TString initialized with the first size characters of the Unicode string \a unicode (encoded as UTF-32).
*/
TString TString::fromWc32(uint32_t unicode) {
    std::string result;
    if(unicode < 0x007F) {
        result += (char)unicode;
    } else if(unicode < 0x07FF) {
        result += (char)(0xC0 + (unicode >> 6));
        result += (char)(0x80 + (unicode & 0x3F));
    } else if(unicode < 0xFFFF) {
        result += (char)(0xE0 + (unicode >> 12));
        result += (char)(0x80 + (unicode >> 6 & 0x3F));
        result += (char)(0x80 + (unicode & 0x3F));
    } else {
        result += (char)(0xF0 + (unicode >> 18));
        result += (char)(0x80 + (unicode >> 12 & 0x3F));
        result += (char)(0x80 + (unicode >> 6 & 0x3F));
        result += (char)(0x80 + (unicode & 0x3F));
    }
    return result;
}
/*!
    Returns a copy of the \a in string. The given string is assumed to be encoded in utf16 if the size of wchar_t is 2 bytes (e.g. on windows).
*/
TString TString::fromWString(const std::wstring &in) {
    TString result;
    for(auto it : in) {
        result += fromWc32(it);
    }
    return result;
}
/*!
    Returns a copy of the \a in string. The given string is assumed to be encoded in UTF-32.
*/
TString TString::fromUtf32(const std::u32string &in) {
    TString result;
    for(auto it : in) {
        result += fromWc32(it);
    }
    return result;
}
/*!
    Returns a std::u32string object with the data contained in this TString.
*/
std::u32string TString::toUtf32() const {
    std::u32string result;

    const char *p = m_data.c_str();
    const char *lim = p + m_data.size();

    uint32_t high;

    uint8_t n = 0;
    for (; p < lim; p += n) {
        // Get number of bytes for one wide character.

        n = 1;	// default: 1 byte. Used when skipping bytes.
        if ((*p & 0x80) == 0) {
            high = (uint32_t)*p;
        } else if ((*p & 0xe0) == 0xc0) {
            n = 2;
            high = (uint32_t)(*p & 0x1f);
        } else if ((*p & 0xf0) == 0xe0) {
            n = 3;
            high = (uint32_t)(*p & 0x0f);
        } else if ((*p & 0xf8) == 0xf0) {
            n = 4;
            high = (uint32_t)(*p & 0x07);
        } else if ((*p & 0xfc) == 0xf8) {
            n = 5;
            high = (uint32_t)(*p & 0x03);
        } else if ((*p & 0xfe) == 0xfc) {
            n = 6;
            high = (uint32_t)(*p & 0x01);
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
        if(n > 1) {
            for(i = 1; i < n; i++) {
                if((p[i] & 0xc0) != 0x80)
                    break;
            }
            if(i != n) {
                n = 1;
                continue;	// skip
            }
        }

        uint32_t out = 0;
        uint32_t n_bits = 0;
        for(i = 1; i < n; i++) {
            out |= (uint32_t)(p[n - i] & 0x3f) << n_bits;
            n_bits += 6;
        }
        out |= high << n_bits;

        result += out;
    }
    return result;
}
/*!
    Returns a copy of this string with the lowest-numbered place-marker %1 replaced by string \a arg1.
*/
TString TString::arg(const TString &arg1) const {
    TString result(*this);
    return result.replace("%1", arg1);
}
/*!
    Returns a copy of this string with the lowest-numbered place-marker %1, %2 replaced by string \a arg1 and \a arg2.
*/
TString TString::arg(const TString &arg1, const TString &arg2) const {
    TString result(*this);
    return result.replace("%1", arg1).replace("%2", arg2);
}
/*!
    Returns a copy of this string with the lowest-numbered place-marker %1, %2, %3 replaced by string \a arg1, \a arg2 and \a arg3.
*/
TString TString::arg(const TString &arg1, const TString &arg2, const TString &arg3) const {
    TString result(*this);
    return result.replace("%1", arg1).replace("%2", arg2).replace("%3", arg3);
}
/*!
    Returns a copy of this string with the lowest-numbered place-marker %1, %2, %3, %4 replaced by string \a arg1, \a arg2, \a arg3 and \a arg4.
*/
TString TString::arg(const TString &arg1, const TString &arg2, const TString &arg3, const TString &arg4) const {
    TString result(*this);
    return result.replace("%1", arg1).replace("%2", arg2).replace("%3", arg3).replace("%4", arg4);
}
/*!
    Returns a copy of this string with the lowest-numbered place-marker %1, %2, %3, %4, %5 replaced by string \a arg1, \a arg2, \a arg3, \a arg4 and \a arg5.
*/
TString TString::arg(const TString &arg1, const TString &arg2, const TString &arg3, const TString &arg4, const TString &arg5) const {
    TString result(*this);
    return result.replace("%1", arg1).replace("%2", arg2).replace("%3", arg3).replace("%4", arg4).replace("%5", arg5);
}
/*!
    Returns a copy of this string with the lowest-numbered place-marker %1, %2, %3, %4, %5, %6 replaced by string \a arg1, \a arg2, \a arg3, \a arg4, \a arg5 and \a arg6.
*/
TString TString::arg(const TString &arg1, const TString &arg2, const TString &arg3, const TString &arg4, const TString &arg5, const TString &arg6) const {
    TString result(*this);
    return result.replace("%1", arg1).replace("%2", arg2).replace("%3", arg3).replace("%4", arg4).replace("%5", arg5).replace("%6", arg6);
}
