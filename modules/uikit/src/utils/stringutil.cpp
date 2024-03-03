#include "utils/stringutil.h"

#include <sstream>
#include <iostream>
#include <algorithm>
#include <cctype>

StringUtil::StringUtil() {

}

StringUtil::~StringUtil() {
}

const char* StringUtil::ws = " \t\n\r\f\v";

std::string StringUtil::longlong2str(long long in) {
    std::stringstream stream;
    stream << in;
    return stream.str();
}

std::string StringUtil::int2str(int in) {
    std::stringstream stream;
    stream << in;
    return stream.str();
}

int StringUtil::str2int(const std::string &in) {
    int out = 0;
    std::stringstream stream(in);
    stream >> out;
    return out;
}

bool StringUtil::startWith(std::string &str, std::string &start) {
    return str.compare(0, start.size(), start) == 0;
}

bool StringUtil::endWith(std::string &str, std::string &end) {
    return str.compare(str.size() - end.size(), end.size(), end) == 0;
}

std::string StringUtil::tolower(const std::string &str) {
    std::string ret(str);
    std::transform(ret.begin(), ret.end(), ret.begin(), ::tolower);
    return ret;
}

std::string StringUtil::toupper(const std::string &str) {
    std::string ret(str);
    std::transform(ret.begin(), ret.end(), ret.begin(), ::toupper);
    return ret;
}

bool StringUtil::contains(const std::string str1, const std::string str2) {
    if (str1.find(str2) != std::string::npos) {
        return true;
    }
    return false;
}

std::string StringUtil::tostring(long long in) {
    std::stringstream ss;
    std::string ret;
    ss << in;
    ss >> ret;
    return ret;
}

void StringUtil::replace(std::string &srcStr, const std::string &findStr,
                               const std::string &replaceStr) {
    std::size_t replaceStrLen = replaceStr.length();
    for (std::size_t pos = 0; pos != std::string::npos; pos += replaceStrLen) {
        if ((pos = srcStr.find(findStr, pos)) != std::string::npos) {
            srcStr.replace(pos, findStr.length(), replaceStr);
        } else {
            break;
        }
    }
}

// trim from end of string (right)
std::string &StringUtil::rtrim(std::string &s, const char *t) {
    s.erase(s.find_last_not_of(t) + 1);
    return s;
}

// trim from beginning of string (left)
std::string &StringUtil::ltrim(std::string &s, const char *t) {
    s.erase(0, s.find_first_not_of(t));
    return s;
}

// trim from both ends of string (right then left)
std::string &StringUtil::trim(std::string &s, const char *t) {
    if (s.empty()) { return s; }
    return rtrim(ltrim(s, t), t);
}

std::vector<std::string> StringUtil::split(const std::string &s, char seperator) {
    std::vector<std::string>container;
    std::istringstream f(s);
    std::istringstream &stream = f;
    std::string out;
    while(std::getline(stream, out, seperator)) {
        container.push_back(out);
    }
    return container;
}

std::vector<std::string> StringUtil::splitButSkipBrackets(const std::string &s, char separator) {
    std::vector<std::string> container;
    size_t length = s.length();
    size_t i = 0, start = 0;
    bool insideBracket = false;

    for (; i < length; i++) {
        if (s[i] == '(' || s[i] == '[' || s[i] == '{') {
            insideBracket = true;
        }
        else if (s[i] == ')' || s[i] == ']' || s[i] == '}') {
            insideBracket = false;
        }
        else if (s[i] == separator && !insideBracket) {
            container.push_back(s.substr(start, i - start));
            start = i + 1;
        }
    }

    container.push_back(s.substr(start));
    return container;
}

std::string StringUtil::join(std::vector<std::string> &v, char separator) {
    std::string s;
    for(unsigned int i = 0; i < v.size(); i++) {
        s += v[i];
        if(i >= (v.size() - 1)) {
            break; // escaping in the last iteration
        }
        s += separator; // concatenating string
    }
    return s;
}

std::string StringUtil::deletechar(const std::string &source, char target) {
    std::string dest = source;
    dest.erase(std::remove(dest.begin(), dest.end(), target));
    return dest;
}
