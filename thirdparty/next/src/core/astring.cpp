#include "astring.h"

#include <algorithm>
#include <iostream>
#include <sstream>

TString::TString() {

}

TString::TString(const ByteArray &array) {
    m_data = std::string(array.begin(), array.end());
}

TString::TString(const std::string &str) :
        m_data(str) {

}

TString::TString(const char *str) :
        m_data(str) {

}

TString::TString(int n, const char ch) {
    m_data.resize(n);
    std::fill(m_data.begin(), m_data.end(), ch);
}

bool TString::operator== (const TString &other) const {
    return m_data == other.m_data;
}

bool TString::operator!= (const TString &other) const {
    return m_data != other.m_data;
}

bool TString::operator< (const TString &other) const {
    return m_data < other.m_data;
}

char TString::operator[] (int i) const {
    return m_data[i];
}

TString TString::operator+ (const TString &other) const {
    return m_data + other.m_data;
}

TString TString::operator+ (const std::string &str) const {
    return m_data + str;
}

TString TString::operator+ (const char *str) const {
    return m_data + str;
}

TString TString::operator+ (char ch) const {
    return m_data + ch;
}

TString &TString::operator+= (const TString &other) {
    m_data += other.m_data;
    return *this;
}

TString &TString::operator+= (const std::string &str) {
    m_data += str;
    return *this;
}

TString &TString::operator+= (const char *str) {
    m_data += str;
    return *this;
}

TString &TString::operator+= (const char ch) {
    m_data += ch;
    return *this;
}

TString &TString::append(const TString &str) {
    m_data.append(str.data());
    return *this;
}

TString &TString::append(const std::string &str) {
    m_data.append(str);
    return *this;
}

TString &TString::append(const char *str) {
    m_data.append(str);
    return *this;
}

TString &TString::append(const char ch, int n) {
    m_data.append(ch, n);
    return *this;
}

char TString::at(int position) const {
    return m_data.at(position);
}

char TString::back() const {
    return m_data.back();
}

void TString::clear() {
    m_data.clear();
}

int TString::compare(const TString &other) const {
    return m_data.compare(other.m_data);
}

bool TString::contains(const TString &str) const {
    return m_data.find(str.data()) != std::string::npos;
}

const char *TString::data() const {
    return m_data.c_str();
}

TString TString::join(const StringList &list, char *separator) {
    TString s;

    int i = 0;
    for(auto it : list) {
        s += it;
        if(i >= (list.size() - 1)) {
            break; // escaping in the last iteration
        }
        i++;
        s += separator; // concatenating string
    }
    return s;
}

bool TString::isEmpty() const {
    return m_data.empty();
}

int TString::indexOf(const TString &str) const {
     return m_data.find(str.m_data);
}

int TString::indexOf(const char ch) const {
    return m_data.find(ch);
}

TString TString::mid(int position, int n) const {
    return m_data.substr(position, n);
}

int TString::lastIndexOf(const TString &str) const {
    return m_data.rfind(str.m_data);
}

int TString::lastIndexOf(const char ch) const {
    return m_data.rfind(ch);
}

int TString::length() const {
    return m_data.length();
}

TString TString::left(int n) const {
    return m_data.substr(0, n);
}

TString TString::number(long long in) {
    std::stringstream stream;
    stream << in;
    return stream.str();
}

TString TString::number(int in) {
    std::stringstream stream;
    stream << in;
    return stream.str();
}

TString TString::number(float in) {
    return std::to_string(in);
}

TString &TString::remove(const TString &str) {
    std::string::size_type i = m_data.find(str.toStdString());

    if(i != std::string::npos) {
        m_data.erase(i, str.length());
    }

    return *this;
}

TString &TString::remove(const char ch) {
    m_data.erase(std::remove(m_data.begin(), m_data.end(), ch));
    return *this;
}

TString &TString::replace(const TString &findStr, const TString &replaceStr) {
    std::size_t replaceStrLen = replaceStr.length();
    for(std::size_t pos = 0; pos != std::string::npos; pos += replaceStrLen) {
        if((pos = m_data.find(findStr.m_data, pos)) != std::string::npos) {
            m_data.replace(pos, findStr.length(), replaceStr.m_data);
        } else {
            break;
        }
    }
    return *this;
}

TString &TString::replace(const char before, const char after) {
    std::replace(m_data.begin(), m_data.end(), before, after);
    return *this;
}

TString &TString::removeFirst() {
    m_data.erase(m_data.begin());
    return *this;
}

TString &TString::removeLast() {
    m_data.erase(m_data.end() - 1);
    return *this;
}

TString TString::right(int n) const {
    return m_data.substr(n, m_data.size() - n);
}

int TString::size() const {
    return m_data.size();
}

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

const std::string &TString::toStdString() const {
    return m_data;
}

TString TString::toLower() const {
    std::string ret(m_data);

    std::transform(ret.begin(), ret.end(), ret.begin(), ::tolower);

    return TString(ret);
}

TString TString::toUpper() const {
    std::string ret(m_data);

    std::transform(ret.begin(), ret.end(), ret.begin(), ::toupper);

    return TString(ret);
}

float TString::toFloat() const {
    return std::stof(m_data);
}

int TString::toInt() const {
    return std::stoi(m_data);
}

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

char TString::front() const {
    return m_data.front();
}

TString TString::fromWc32(uint32_t wc32) {
    std::string result;
    if(wc32 < 0x007F) {
        result += (char)wc32;
    } else if(wc32 < 0x07FF) {
        result += (char)(0xC0 + (wc32 >> 6));
        result += (char)(0x80 + (wc32 & 0x3F));
    } else if(wc32 < 0xFFFF) {
        result += (char)(0xE0 + (wc32 >> 12));
        result += (char)(0x80 + (wc32 >> 6 & 0x3F));
        result += (char)(0x80 + (wc32 & 0x3F));
    } else {
        result += (char)(0xF0 + (wc32 >> 18));
        result += (char)(0x80 + (wc32 >> 12 & 0x3F));
        result += (char)(0x80 + (wc32 >> 6 & 0x3F));
        result += (char)(0x80 + (wc32 & 0x3F));
    }
    return result;
}

TString TString::fromWString(const std::wstring &in) {
    TString result;
    for(auto it : in) {
        result += fromWc32(it);
    }
    return result;
}

TString TString::fromUtf32(const std::u32string &in) {
    TString result;
    for(auto it : in) {
        result += fromWc32(it);
    }
    return result;
}

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
