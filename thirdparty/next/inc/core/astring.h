/*
    This file is part of Thunder Next.

    Thunder Next is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

    Thunder Next is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Thunder Next.  If not, see <http://www.gnu.org/licenses/>.

    Copyright: 2008-2025 Evgeniy Prikazchikov
*/

#ifndef ASTRING_H
#define ASTRING_H

#include <list>
#include <string>
#include <vector>

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

    char &operator[] (int i);

    TString operator+ (const TString &other) const;
    TString operator+ (const std::string &str) const;
    TString operator+ (const char *str) const;
    TString operator+ (char ch) const;

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

    int indexOf(const TString &str) const;
    int indexOf(const char ch) const;

    TString mid(int position, int n) const;

    int lastIndexOf(const TString &str) const;
    int lastIndexOf(const char ch) const;

    int length() const;

    TString left(int n) const;

    TString &remove(const TString &str);
    TString &remove(const char ch);

    TString &replace(const TString &findStr, const TString &replaceStr);
    TString &replace(const char before, const char after);

    TString &removeFirst();
    TString &removeLast();

    TString right(int n) const;

    int size() const;

    StringList split(const char sep) const;
    StringList split(const TString &sep) const;

    const std::string &toStdString() const;

    TString toLower() const;

    TString toUpper() const;

    float toFloat() const;
    int toInt() const;
    size_t toLong() const;

    std::u32string toUtf32() const;

    TString trimmed() const;
    TString trim(const char *t) const;

    char front() const;

    static TString number(long long in);
    static TString number(int in);
    static TString number(float in);

    static TString fromWc32(uint32_t wc32);
    static TString fromWString(const std::wstring &in);
    static TString fromUtf32(const std::u32string &in);

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
