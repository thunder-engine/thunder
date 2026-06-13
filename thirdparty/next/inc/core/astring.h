/*
    This file is part of Thunder Next.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#ifndef ASTRING_H
#define ASTRING_H

#include <list>
#include <string>
#include <vector>
#include <cstdint>

#include <global.h>

class TString;

typedef std::vector<uint8_t> ByteArray;

typedef std::list<TString> StringList;

class NEXT_LIBRARY_EXPORT TString {

public:
    TString();

    TString(const ByteArray &array);
    TString(const std::string &str);
    TString(const char *str);
    TString(int n, const char ch);

    bool operator== (const TString &other) const;
    bool operator!= (const TString &other) const;

    bool operator< (const TString &other) const;

    char &operator[] (int position);

    TString operator+ (const TString &other) const;
    TString operator+ (const std::string &other) const;
    TString operator+ (const char *other) const;
    TString operator+ (char character) const;

    TString &operator+= (const TString &other);
    TString &operator+= (const std::string &str);
    TString &operator+= (const char *str);
    TString &operator+= (const char other);

    TString &append(const TString &str);
    TString &append(const std::string &str);
    TString &append(const char *str);
    TString &append(const char ch, int n);

    char at(int position) const;

    char back() const;

    void clear();

    int compare(const TString &other) const;

    bool contains(const TString &str) const;

    const char *data() const;

    static TString join(const StringList &list, const char *separator);

    bool isEmpty() const;

    bool startsWith(const TString &str) const;

    int indexOf(const TString &str) const;
    int indexOf(const char ch) const;

    TString mid(int position, int n) const;

    int lastIndexOf(const TString &str) const;
    int lastIndexOf(const char ch) const;

    int length() const;

    TString left(int n) const;

    TString &remove(const TString &str);
    TString &remove(const char ch);

    TString &replace(const TString &before, const TString &after);
    TString &replace(const char before, const char after);

    TString &removeFirst();
    TString &removeLast();

    TString right(int n) const;

    int size() const;

    StringList split(const char sep) const;
    StringList split(const TString &sep) const;

    const std::string &toStdString() const;
    std::wstring toStdWString() const;

    TString toLower() const;

    TString toUpper() const;

    float toFloat() const;
    int toInt() const;
    size_t toLong() const;

    std::u32string toUtf32() const;

    TString trimmed() const;
    TString trim(const char *t) const;

    TString simplified() const;

    char front() const;

    static TString number(long long in);
    static TString number(int in);
    static TString number(float in);

    static TString fromWc32(uint32_t unicode);
    static TString fromWString(const std::wstring &in);
    static TString fromUtf32(const std::u32string &in);

    TString arg(const TString &arg1) const;
    TString arg(const TString &arg1, const TString &arg2) const;
    TString arg(const TString &arg1, const TString &arg2, const TString &arg3) const;
    TString arg(const TString &arg1, const TString &arg2, const TString &arg3, const TString &arg4) const;
    TString arg(const TString &arg1, const TString &arg2, const TString &arg3, const TString &arg4, const TString &arg5) const;
    TString arg(const TString &arg1, const TString &arg2, const TString &arg3, const TString &arg4, const TString &arg5, const TString &arg6) const;

private:
    std::string m_data;

};

template <>
struct std::hash<TString> {
    std::size_t operator()(const TString &k) const {
        return std::hash<std::string>()(k.toStdString());
    }
};

#endif // ASTRING_H
