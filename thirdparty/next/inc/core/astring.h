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
#include <macros.h>

class String;

typedef std::vector<uint8_t> ByteArray;

typedef std::list<String> StringList;

class NEXT_LIBRARY_EXPORT String {

public:
    String();

    String(const ByteArray &array);
    String(const std::string &str);
    String(const char *str);
    String(int n, const char ch);

    bool operator== (const String &other) const;
    bool operator!= (const String &other) const;

    bool operator< (const String &other) const;

    String operator+ (const String &other);
    String operator+ (const std::string &str);
    String operator+ (const char *str);
    String operator+ (char ch) const;

    String &operator+= (const String &other);
    String &operator+= (const std::string &str);
    String &operator+= (const char *str);
    String &operator+= (const char other);

    String &append(const String &str);
    String &append(const std::string &str);
    String &append(const char *str);
    String &append(const char ch, int n);

    char at(int position) const;

    char back() const;

    void clear();

    int compare(const String &other) const;

    bool contains(const String &str) const;

    const char *data() const;

    String join(const StringList &list, char separator) const;

    bool isEmpty() const;

    int indexOf(const String &str) const;
    int indexOf(const char ch) const;

    String mid(int position, int n) const;

    int lastIndexOf(const String &str) const;
    int lastIndexOf(const char ch) const;

    int length() const;

    String left(int n);

    String &remove(const String &str);
    String &remove(const char ch);

    String &replace(const String &findStr, const String &replaceStr);
    String &replace(const char before, const char after);

    String &removeFirst();
    String &removeLast();

    String right(int n);

    int size() const;

    StringList split(const char sep) const;
    StringList split(const String &sep) const;

    const std::string &toStdString() const;

    String toLower() const;

    String toUpper() const;

    float toFloat() const;
    int toInt() const;
    size_t toLong() const;

    std::u32string toUtf32() const;

    String trimmed() const;
    String trim(const char *t) const;

    char front() const;

    static String number(long long in);
    static String number(int in);
    static String number(float in);

    static String fromWc32(uint32_t wc32);
    static String fromWString(const std::wstring &in);
    static String fromUtf32(const std::u32string &in);

private:
    std::string m_data;

};

template <>
struct std::hash<String> {
    std::size_t operator()(const String &k) const {
        return std::hash<std::string>()(k.toStdString());
    }
};

#endif // ASTRING_H
