#include "core/ajson.h"

#include "core/avariant.h"
#include "core/aobjectsystem.h"

#define J_TRUE  "true"
#define J_FALSE "false"
#define J_NULL  "null"

#define FORMAT (depth > -1) ? "\n" : "";

typedef stack<AVariant> VariantStack;
typedef stack<string>   NameStack;

void appendProperty(VariantStack &s, const AVariant &data, const string &name) {
    AVariant v;
    if(!s.empty()) {
        v   = s.top();
        s.pop();
    }
    switch(v.type()) {
        case AMetaType::VARIANTLIST: {
            AVariantList list = v.value<AVariantList>();
            list.push_back(data);
            s.push(list);
            return;
        }
        case AMetaType::VARIANTMAP: {
            AVariantMap map   = v.value<AVariantMap>();
            map[name]    = data;
            s.push(map);
            return;
        } break;
        default: break;
    }
    s.push(v);
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

AJson::AJson() {
    PROFILE_FUNCTION()
}

AVariant AJson::load(const string &data) {
    PROFILE_FUNCTION()
    AVariant result;

    VariantStack    s;
    NameStack       n;
    string name;
    States state    = propertyValue;
    uint32_t it     = 0;
    while(it < data.length()) {
        skipSpaces(data.c_str(), it);
        switch(data[it]) {
            case '{': {
                AVariantMap map;
                s.push(map);
                n.push(name);
                name    = "";
                state   = propertyName;
            } break;
            case '}': {
                result  = s.top();
                s.pop();
                if(s.size()) {
                    appendProperty(s, result, n.top());
                }
                n.pop();
                state   = propertyName;
            } break;
            case '[': {
                if(state == propertyValue) {
                    AVariantList list;
                    s.push(list);
                    n.push(name);
                    name    = "";
                }
            } break;
            case ']': {
                AVariantList list   = s.top().value<AVariantList>();
                s.pop();
                uint32_t type   = list.front().toInt();
                list.pop_front();
                if(type != AMetaType::VARIANTLIST) {
                    void *object    = AMetaType::create(type);
                    AMetaType::convert(&list, AMetaType::VARIANTLIST, object, type);
                    appendProperty(s, AVariant(type, object), n.top());
                } else {
                    appendProperty(s, list, n.top());
                }
                result  = list;
                n.pop();
                state   = propertyName;
            } break;
            case ':': {
                state   = propertyValue;
            } break;
            case ',': {
                if(s.top().type() == AMetaType::VARIANTLIST) {
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
                    AVariant v(data.substr(st, it - st));
                    appendProperty(s, (number) ? AVariant(v.toDouble()) : AVariant(v.toInt()), name);
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

string AJson::save(const AVariant &data, int32_t depth) {
    PROFILE_FUNCTION()
    string result;
    switch(data.type()) {
        case AMetaType::BOOLEAN:
        case AMetaType::DOUBLE:
        case AMetaType::INTEGER: {
            result += data.toString();
        } break;
        case AMetaType::STRING: {
            result += '"' + data.toString() + '"';
        } break;
        case AMetaType::VARIANTMAP: {
            result += "{";
            result += FORMAT;
            uint32_t i = 1;
            AVariantMap map = data.toMap();
            for(auto &it: map) {
                result.append(depth + 1, '\t');
                result += "\"" + it.first + "\":" + ((depth > -1) ? " " : "") + save(it.second, (depth > -1) ? depth + 1 : depth);
                result += ((i < map.size()) ? "," : "");
                result += FORMAT;
                i++;
            }
            if(depth > -1) {
                result.append(depth, '\t');
            }
            result += "}";
        } break;
        default: {
            result += "[";
            result += FORMAT;
            uint32_t i = 1;
            AVariantList list = data.toList();
            for(auto &it: list) {
                result.append(depth + 1, '\t');
                result += save(it, (depth > -1) ? depth + 1 : depth);
                result += ((i < list.size()) ? "," : "");
                result += FORMAT;
                i++;
            }
            if(depth > -1) {
                result.append(depth, '\t');
            }
            result += "]";
        } break;
    }
    return result;
}

inline string AJson::readString(const string &data, uint32_t &it) {
    PROFILE_FUNCTION()
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

inline void AJson::skipSpaces(const char *data, uint32_t &it) {
    PROFILE_FUNCTION()
    while(isSpace(data[it])) {
        it++;
    }
}

inline bool AJson::isSpace(uint8_t c) {
    PROFILE_FUNCTION()
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

inline bool AJson::isDigit(uint8_t c) {
    PROFILE_FUNCTION()
    return c >= '0' && c <= '9';
}
