#include "core/json.h"

#include "core/variant.h"
#include "core/objectsystem.h"

#define J_TRUE  "true"
#define J_FALSE "false"
#define J_NULL  "null"

#define FORMAT (tab > -1) ? "\n" : "";

typedef stack<Variant>  VariantStack;
typedef stack<string>   NameStack;

void appendProperty(VariantStack &s, const Variant &data, const string &name) {
    Variant v;
    if(!s.empty()) {
        v   = s.top();
        s.pop();
    }
    switch(v.type()) {
        case MetaType::VARIANTLIST: {
            VariantList list = v.value<VariantList>();
            list.push_back(data);
            s.push(list);
            return;
        }
        case MetaType::VARIANTMAP: {
            VariantMap map  = v.value<VariantMap>();
            uint32_t type   = MetaType::type(name.c_str());
            if((type >= MetaType::VECTOR2 && type < MetaType::USERTYPE)) {
                Variant object(type, MetaType::create(type));
                VariantList list    = data.toList();
                MetaType::convert(&list, MetaType::VARIANTLIST, object.data(), type);
                s.push(object);
            } else {
                map[name]    = data;
                s.push(map);
            }
            return;
        } break;
        default: break;
    }
    s.push(v);
}


inline string readString(const string &data, uint32_t &it) {
    PROFILE_FUNCTION();
    uint32_t s  = ++it;
    char c      = data[s];
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
        dictionary["bool"]    = true;
        dictionary["str"]     = "string";
        dictionary["int"]     = 1;
        dictionary["float"]   = 2.0f;

        string data = Json::save(dictionary); // Serializing dictionary to string
        ....
        VariantMap result   = Json::load(data).toMap(); // Resotoring it back
    \endcode
*/
/*!
    Returns deserialized string \a data as Variant based DOM structure.
*/
Variant Json::load(const string &data) {
    PROFILE_FUNCTION();
    Variant result;

    VariantStack    s;
    NameStack       n;
    string name;
    States state    = propertyValue;
    uint32_t it     = 0;
    while(it < data.length()) {
        skipSpaces(data.c_str(), it);
        switch(data[it]) {
            case '{': {
                VariantMap map;
                s.push(map);
                n.push(name);
                name    = "";
                state   = propertyName;
            } break;
            case '}': {
                result  = s.top();
                s.pop();
                if(!s.empty()) {
                    appendProperty(s, result, n.top());
                }
                n.pop();
                state   = propertyName;
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
                result    = s.top();
                s.pop();
                if(!s.empty()) {
                    appendProperty(s, result, n.top());
                }
                n.pop();
                state   = propertyName;
            } break;
            case ':': {
                state   = propertyValue;
            } break;
            case ',': {
                if(s.top().type() == MetaType::VARIANTLIST) {
                    state   = propertyValue;
                } else {
                    state   = propertyName;
                    name    = "";
                }
            } break;
            case '"': {
                string str  = readString(data, it);
                if(state == propertyName) {
                    name    = str;
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
                while(it < data.length()) {
                    char c  = data[++it];
                    if(!isDigit(c) && c != '.') {
                        break;
                    }
                    if(c == '.') {
                        number  = true;
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
                    it  += 3;
                }
            } break;
            case 'f': {
                if(data.substr(it, 5) == J_FALSE) {
                    if(state == propertyValue) {
                        appendProperty(s, false, name);
                    }
                    it  += 4;
                }
            } break;
            case 'n': {
                if(data.substr(it, 4) == J_NULL) {
                    if(state == propertyValue) {
                        appendProperty(s, static_cast<void *>(nullptr), name);
                    }
                    it  += 3;
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
string Json::save(const Variant &data, int32_t tab) {
    PROFILE_FUNCTION();
    string result;
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
            uint32_t i = 1;
            if(type >= MetaType::VECTOR2 && type < MetaType::USERTYPE) {
                result.append(tab + 1, '\t');
                result += (string("\"") + MetaType::name(type) + "\":");
                result += FORMAT;
                result.append(tab + 1, '\t');
                result += save(data.toList(), (tab > -1) ? tab + 1 : tab);
                result += FORMAT;
            } else {
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
