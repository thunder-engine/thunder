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

#include "core/json.h"

#include <list>
#include <map>
#include <stack>

#include "core/variant.h"
#include "core/objectsystem.h"

#define J_TRUE  "true"
#define J_FALSE "false"
#define J_NULL  "null"

#define FORMAT (tab > -1) ? "\n" : ""

typedef std::stack<Variant> VariantStack;
typedef std::stack<std::string> NameStack;

void appendProperty(VariantStack &s, const Variant &data, const std::string &name) {
    Variant v;
    if(!s.empty()) {
        v = s.top();
        s.pop();
    }
    switch(v.type()) {
        case MetaType::VARIANTLIST: {
            VariantList &list = *(reinterpret_cast<VariantList *>(v.data()));
            list.push_back(data);
            s.push(list);
            return;
        }
        case MetaType::VARIANTMAP: {
            uint32_t type = MetaType::type(name.c_str());
            if((type >= MetaType::VECTOR2 && type < MetaType::USERTYPE)) {
                Variant object(type, MetaType::create(type));
                VariantList &list = *(reinterpret_cast<VariantList *>(data.data()));
                MetaType::convert(&list, MetaType::VARIANTLIST, object.data(), type);
                s.push(object);
            } else {
                VariantMap &map = *(reinterpret_cast<VariantMap *>(v.data()));
                map[name] = data;
                s.push(map);
            }
            return;
        }
        default: break;
    }
    s.push(v);
}


inline std::string readString(const std::string &data, uint32_t &it) {
    PROFILE_FUNCTION();
    uint32_t s  = ++it;
    char c = data[s];
    while(it < data.length() && c != '"') {
        c = data[++it];
        if(c == '\\') {
            c = data[++it];
        }
    }
    return data.substr(s, it - s);
}

inline bool isSpace(uint8_t c) {
    PROFILE_FUNCTION();
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

inline void skipSpaces(const char *data, uint32_t &it) {
    PROFILE_FUNCTION();
    while(isSpace(data[it])) {
        it++;
    }
}

inline bool isDigit(uint8_t c) {
    PROFILE_FUNCTION();
    return c >= '0' && c <= '9';
}

enum States {
    objectBegin = 1,
    objectEnd,
    arrayBegin,
    arrayEnd,
    propertyNext,
    propertyName,
    propertyValue
};
/*!
    \class Json
    \brief JSON format parser.
    \since Next 1.0
    \inmodule Core

    This class implements Json parser with Variant based DOM structure input/output.
    It allows to serialize and deserialize object structures represented in Variant DOM structure.

    Example:
    \code
        VariantMap dictionary;
        dictionary["bool"]  = true;
        dictionary["str"]   = "string";
        dictionary["int"]   = 1;
        dictionary["float"] = 2.0f;

        string data = Json::save(dictionary); // Serializing dictionary to string
        ....
        VariantMap result = Json::load(data).toMap(); // Resotoring it back
    \endcode
*/
/*!
    Returns deserialized string \a data as Variant based DOM structure.
*/
Variant Json::load(const std::string &data) {
    PROFILE_FUNCTION();
    Variant result;

    VariantStack s;
    NameStack    n;
    std::string name;
    States state = propertyValue;
    uint32_t it  = 0;
    while(it < data.length()) {
        skipSpaces(data.c_str(), it);
        switch(data[it]) {
            case '{': {
                VariantMap map;
                s.push(map);
                n.push(name);
                name  = "";
                state = propertyName;
            } break;
            case '}': {
                result = s.top();
                s.pop();
                if(!s.empty()) {
                    appendProperty(s, result, n.top());
                }
                n.pop();
                state = propertyName;
            } break;
            case '[': {
                if(state == propertyValue) {
                    VariantList list;
                    s.push(list);
                    n.push(name);
                    name    = "";
                }
            } break;
            case ']': {
                result = s.top();
                s.pop();
                if(!s.empty()) {
                    appendProperty(s, result, n.top());
                }
                n.pop();
                state = propertyName;
            } break;
            case ':': {
                state   = propertyValue;
            } break;
            case ',': {
                if(s.top().type() == MetaType::VARIANTLIST) {
                    state = propertyValue;
                } else {
                    state = propertyName;
                    name = "";
                }
            } break;
            case '"': {
                std::string str = readString(data, it);
                if(state == propertyName) {
                    name = str;
                } else {
                    appendProperty(s, str, name);
                }
            } break;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            case '-': {
                uint32_t st = it;
                bool number = false;
                bool enotation = false;
                while(it < data.length()) {
                    char c = data[++it];
                    if(!isDigit(c) && c != '.') {
                        if(c != 'e' && c != 'E') {
                            if(enotation) {
                                if(c != '+' && c != '-') {
                                    break;
                                }
                            } else {
                                break;
                            }
                        } else {
                            enotation = true;
                        }
                    }
                    if(c == '.') {
                        number = true;
                    }
                }
                if(state == propertyValue) {
                    Variant v(data.substr(st, it - st));
                    appendProperty(s, (number) ? Variant(v.toFloat()) : Variant(v.toInt()), name);
                }
                it--;
            } break;
            case 't': {
                if(data.substr(it, 4) == J_TRUE) {
                    if(state == propertyValue) {
                        appendProperty(s, true, name);
                    }
                    it += 3;
                }
            } break;
            case 'f': {
                if(data.substr(it, 5) == J_FALSE) {
                    if(state == propertyValue) {
                        appendProperty(s, false, name);
                    }
                    it += 4;
                }
            } break;
            case 'n': {
                if(data.substr(it, 4) == J_NULL) {
                    if(state == propertyValue) {
                        appendProperty(s, static_cast<void *>(nullptr), name);
                    }
                    it += 3;
                }
            } break;
            default: {
                return result;
            }
        }
        it++;
    }
    return result;
}
/*!
    Returns serialized \a data as string.
    Argument \a tab is used as JSON tabulation formatting offset (-1 for one line JSON)
*/
std::string Json::save(const Variant &data, int32_t tab) {
    PROFILE_FUNCTION();
    std::string result;
    uint32_t type   = data.type();
    switch(type) {
        case MetaType::BOOLEAN:
        case MetaType::FLOAT:
        case MetaType::INTEGER: {
            result += data.toString();
        } break;
        case MetaType::STRING: {
            result += '"' + data.toString() + '"';
        } break;
        case MetaType::VARIANTLIST: {
            result += "[";
            result += FORMAT;
            uint32_t i  = 1;
            VariantList list = data.toList();
            for(auto &it: list) {
                result.append(tab + 1, '\t');
                result += save(it, (tab > -1) ? tab + 1 : tab);
                result += ((i < list.size()) ? "," : "");
                result += FORMAT;
                i++;
            }
            if(tab > -1) {
                result.append(tab, '\t');
            }
            result += "]";
        } break;
        default: {
            result += "{";
            result += FORMAT;
            if(type >= MetaType::VECTOR2 && type < MetaType::USERTYPE) {
                result.append(tab + 1, '\t');
                result += (std::string("\"") + MetaType::name(type) + "\":");
                result += FORMAT;
                result.append(tab + 1, '\t');
                result += save(data.toList(), (tab > -1) ? tab + 1 : tab);
                result += FORMAT;
            } else {
                uint32_t i = 1;
                VariantMap map = data.toMap();
                for(auto &it: map) {
                    result.append(tab + 1, '\t');
                    result += "\"" + it.first + "\":" + ((tab > -1) ? " " : "") + save(it.second, (tab > -1) ? tab + 1 : tab);
                    result += ((i < map.size()) ? "," : "");
                    result += FORMAT;
                    i++;
                }
            }
            if(tab > -1) {
                result.append(tab, '\t');
            }
            result += "}";
        } break;
    }
    return result;
}
