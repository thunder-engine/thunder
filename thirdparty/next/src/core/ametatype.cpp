#include "core/ametatype.h"

#include <list>

#include "math/amath.h"
#include "core/avariant.h"



#define DECLARE_BUILT_TYPE(TYPE) \
    { \
        TypeFuncs<TYPE>::size, \
        TypeFuncs<TYPE>::static_new, \
        TypeFuncs<TYPE>::construct, \
        TypeFuncs<TYPE>::static_delete, \
        TypeFuncs<TYPE>::destruct, \
        TypeFuncs<TYPE>::clone, \
        TypeFuncs<TYPE>::move, \
        TypeFuncs<TYPE>::compare, \
        TypeFuncs<TYPE>::index, \
        #TYPE \
    }

typedef map<uint32_t, AMetaType::Table> TypeMap;
typedef map<string, uint32_t>           NameMap;
typedef map<uint32_t, map<uint32_t, AMetaType::converterCallback>> ConverterMap;

uint32_t AMetaType::s_NextId = AMetaType::USERTYPE;
static TypeMap s_Types = {
    {AMetaType::BOOLEAN,    DECLARE_BUILT_TYPE(bool)},
    {AMetaType::INTEGER,    DECLARE_BUILT_TYPE(int)},
    {AMetaType::DOUBLE,     DECLARE_BUILT_TYPE(double)},
    {AMetaType::STRING,     DECLARE_BUILT_TYPE(string)},
    {AMetaType::VARIANTMAP, DECLARE_BUILT_TYPE(AVariantMap)},
    {AMetaType::VARIANTLIST,DECLARE_BUILT_TYPE(AVariantList)},
    {AMetaType::BYTEARRAY,  DECLARE_BUILT_TYPE(AByteArray)},
    {AMetaType::VECTOR2,    DECLARE_BUILT_TYPE(Vector2)},
    {AMetaType::VECTOR3,    DECLARE_BUILT_TYPE(Vector3)},
    {AMetaType::VECTOR4,    DECLARE_BUILT_TYPE(Vector4)},
    {AMetaType::QUATERNION, DECLARE_BUILT_TYPE(Quaternion)},
    {AMetaType::MATRIX3,    DECLARE_BUILT_TYPE(Matrix3)},
    {AMetaType::MATRIX4,    DECLARE_BUILT_TYPE(Matrix4)}
};

static ConverterMap s_Converters= {
    {AMetaType::BOOLEAN,   {{AMetaType::INTEGER,    &AMetaType::toBoolean},
                            {AMetaType::DOUBLE,     &AMetaType::toBoolean},
                            {AMetaType::STRING,     &AMetaType::toBoolean}}},

    {AMetaType::INTEGER,   {{AMetaType::BOOLEAN,    &AMetaType::toInteger},
                            {AMetaType::DOUBLE,     &AMetaType::toInteger},
                            {AMetaType::STRING,     &AMetaType::toInteger}}},

    {AMetaType::DOUBLE,    {{AMetaType::BOOLEAN,    &AMetaType::toDouble},
                            {AMetaType::INTEGER,    &AMetaType::toDouble},
                            {AMetaType::STRING,     &AMetaType::toDouble}}},

    {AMetaType::STRING,    {{AMetaType::BOOLEAN,    &AMetaType::toString},
                            {AMetaType::INTEGER,    &AMetaType::toString},
                            {AMetaType::DOUBLE,     &AMetaType::toString}}},

    {AMetaType::VARIANTLIST,{{AMetaType::VECTOR2,   &AMetaType::toList},
                             {AMetaType::VECTOR3,   &AMetaType::toList},
                             {AMetaType::VECTOR4,   &AMetaType::toList},
                             {AMetaType::QUATERNION,&AMetaType::toList},
                             {AMetaType::MATRIX3,   &AMetaType::toList},
                             {AMetaType::MATRIX4,   &AMetaType::toList}}},

    {AMetaType::VECTOR2,    {{AMetaType::INTEGER,   &AMetaType::toVector2},
                             {AMetaType::DOUBLE,    &AMetaType::toVector2},
                             {AMetaType::VARIANTLIST,&AMetaType::toVector2}}},

    {AMetaType::VECTOR3,    {{AMetaType::INTEGER,   &AMetaType::toVector3},
                             {AMetaType::DOUBLE,    &AMetaType::toVector3},
                             {AMetaType::VARIANTLIST,&AMetaType::toVector3},
                             {AMetaType::VECTOR2,  &AMetaType::toVector3}}},

    {AMetaType::VECTOR4,    {{AMetaType::INTEGER,   &AMetaType::toVector4},
                             {AMetaType::DOUBLE,    &AMetaType::toVector4},
                             {AMetaType::VARIANTLIST,&AMetaType::toVector4},
                             {AMetaType::VECTOR2,  &AMetaType::toVector4},
                             {AMetaType::MATRIX3,  &AMetaType::toVector4}}},

    {AMetaType::QUATERNION, {{AMetaType::VARIANTLIST,&AMetaType::toQuaternion},
                             {AMetaType::MATRIX3,  &AMetaType::toQuaternion}}},

    {AMetaType::MATRIX3,    {{AMetaType::VARIANTLIST,&AMetaType::toMatrix3}}},

    {AMetaType::MATRIX4,    {{AMetaType::VARIANTLIST,&AMetaType::toMatrix4}}}
};

static NameMap s_Names = {
    {"bool",           AMetaType::BOOLEAN},
    {"int",            AMetaType::INTEGER},
    {"double",         AMetaType::DOUBLE},
    {"string",         AMetaType::STRING},
    {"map",            AMetaType::VARIANTMAP},
    {"list",           AMetaType::VARIANTLIST},
    {"AByteArray",     AMetaType::BYTEARRAY},
    {"Vector2",       AMetaType::VECTOR2},
    {"Vector3",       AMetaType::VECTOR3},
    {"Vector4",       AMetaType::VECTOR4},
    {"Quaternion",     AMetaType::QUATERNION},
    {"Matrix3",       AMetaType::MATRIX3},
    {"Matrix4",       AMetaType::MATRIX4}
};

AMetaType::AMetaType(const Table *table) :
        m_pTable(table) {
    PROFILE_FUNCTION()
}

const char *AMetaType::name() const {
    PROFILE_FUNCTION()
    return m_pTable->name;
}

int AMetaType::size() const {
    PROFILE_FUNCTION()
    return m_pTable->get_size();
}

void *AMetaType::construct(void *where, const void *copy) const {
    PROFILE_FUNCTION()
    if(copy) {
        m_pTable->clone(&copy, &where);
        return where;
    } else {
        m_pTable->construct(&where);
        return where;
    }
}

void *AMetaType::create(const void *copy) const {
    PROFILE_FUNCTION()
    void *where = nullptr;
    m_pTable->static_new(&where);

    if(copy) {
        m_pTable->clone(&copy, &where);
    }
    return where;
}

void AMetaType::destroy(void *data) const {
    PROFILE_FUNCTION()
    m_pTable->static_delete(&data);
}

void AMetaType::destruct(void *data) const {
    PROFILE_FUNCTION()
    m_pTable->destruct(&data);
}

bool AMetaType::compare(const void *left, const void *right) const {
    PROFILE_FUNCTION()
    return m_pTable->compare(&left, &right);
}

bool AMetaType::isValid() const {
    PROFILE_FUNCTION()
    return (m_pTable != nullptr);
}

uint32_t AMetaType::registerType(Table &table) {
    PROFILE_FUNCTION()
    uint32_t result = ++AMetaType::s_NextId;
    s_Types[result] = table;
    s_Names[table.name] = result;
    return result;
}

uint32_t AMetaType::type(const char *name) {
    PROFILE_FUNCTION()
    auto it = s_Names.find(name);
    if(it != s_Names.end()) {
        return it->second;
    }
    return INVALID;
}
uint32_t AMetaType::type(const type_info &type) {
    PROFILE_FUNCTION()
    for(auto it : s_Types) {
        if(it.second.index() == type_index(type) ) {
            return it.first;
        }
    }
    return INVALID;
}

const char *AMetaType::name(uint32_t type) {
    PROFILE_FUNCTION()
    auto it = s_Types.find(type);
    if(it != s_Types.end()) {
        return AMetaType(&(it->second)).name();
    }
    return nullptr;
}

int AMetaType::size(uint32_t type) {
    PROFILE_FUNCTION()
    auto it = s_Types.find(type);
    if(it != s_Types.end()) {
        return AMetaType(&(it->second)).size();
    }
    return 0;
}

void *AMetaType::construct(uint32_t type, void *where, const void *copy) {
    PROFILE_FUNCTION()
    auto it = s_Types.find(type);
    if(it != s_Types.end()) {
        return AMetaType(&(it->second)).construct(where, copy);
    }
    return nullptr;
}

void *AMetaType::create(uint32_t type, const void *copy) {
    PROFILE_FUNCTION()
    auto it = s_Types.find(type);
    if(it != s_Types.end()) {
        return AMetaType(&(it->second)).create(copy);
    }
    return nullptr;
}

void AMetaType::destroy(uint32_t type, void *data) {
    PROFILE_FUNCTION()
    auto it = s_Types.find(type);
    if(it != s_Types.end()) {
        AMetaType(&(it->second)).destroy(data);
    }
}

void AMetaType::destruct(uint32_t type, void *data) {
    PROFILE_FUNCTION()
    auto it = s_Types.find(type);
    if(it != s_Types.end()) {
        AMetaType(&(it->second)).destruct(data);
    }
}

bool AMetaType::compare(const void *left, const void *right, uint32_t type) {
    PROFILE_FUNCTION()
    auto it = s_Types.find(type);
    if(it != s_Types.end()) {
        return AMetaType(&(it->second)).compare(left, right);
    }
    return false;
}

bool AMetaType::convert(const void *from, uint32_t fromType, void *to, uint32_t toType) {
    PROFILE_FUNCTION()
    auto t = s_Converters.find(toType);
    if(t != s_Converters.end()) {
        auto it = t->second.find(fromType);
        if(it != t->second.end()) {
            return (*it->second)(to, from, fromType);
        }
    }
    return false;
}

bool AMetaType::registerConverter(uint32_t from, uint32_t to, converterCallback function) {
    PROFILE_FUNCTION()
    if(hasConverter(from, to)) {
        return false;
    }

    auto t = s_Converters.find(to);
    if(t != s_Converters.end()) {
        t->second[from] = function;
    } else {
        map<uint32_t, converterCallback> m;
        m[from] = function;
        s_Converters[to]    = m;
    }
    return true;
}

bool AMetaType::hasConverter(uint32_t from, uint32_t to) {
    PROFILE_FUNCTION()
    auto t = s_Converters.find(to);
    if(t != s_Converters.end()) {
        auto it = t->second.find(from);
        if(it != t->second.end()) {
            return true;
        }
    }
    return false;
}

bool AMetaType::toBoolean(void *to, const void *from, const uint32_t fromType) {
    PROFILE_FUNCTION()
    bool result = true;
    bool *r     = static_cast<bool *>(to);
    switch(fromType) {
        case INTEGER: { *r  = *(static_cast<const int *>(from)) != 0; } break;
        case DOUBLE:  { *r  = *(static_cast<const double *>(from)) != 0; } break;
        case STRING:  {
            string s  = *(static_cast<const string *>(from));
            *r = (s != "false" || s != "0" || !s.empty());
        }  break;
        default:      { result  = false; } break;
    }
    return result;
}

bool AMetaType::toInteger(void *to, const void *from, const uint32_t fromType) {
    PROFILE_FUNCTION()
    bool result = true;
    int *r      = static_cast<int *>(to);
    switch(fromType) {
        case BOOLEAN: { *r      = (*(static_cast<const bool *>(from))) ? 1 : 0; } break;
        case DOUBLE: { double f = *(static_cast<const double *>(from)); *r = int(f); f -= *r; *r += (f >= 0.5f) ? 1 : 0; } break;
        case STRING: {
            string s  = *(static_cast<const string *>(from));
            char *end;
            *r        = strtol(s.c_str(), &end, 10);
        } break;
        default:    { result    = false; } break;
    }
    return result;
}

bool AMetaType::toDouble(void *to, const void *from, const uint32_t fromType) {
    PROFILE_FUNCTION()
    bool result = true;
    double *r   = static_cast<double *>(to);
    switch(fromType) {
        case BOOLEAN: { *r  = double(*(static_cast<const bool *>(from))); } break;
        case INTEGER: { *r  = double(*(static_cast<const int *>(from))); } break;
        case STRING:  {
            string s    = *(static_cast<const string *>(from));
            char *end;
            *r          = strtod(s.c_str(), &end);
        } break;
        default:      { result  = false; } break;
    }
    return result;
}

bool AMetaType::toString(void *to, const void *from, const uint32_t fromType) {
    PROFILE_FUNCTION()
    bool result = true;
    string *r   = static_cast<string *>(to);
    switch(fromType) {
        case BOOLEAN: { *r        = (*(static_cast<const bool *>(from))) ? "true" : "false"; } break;
        case DOUBLE:  { string s  = to_string(*(static_cast<const double *>(from))); *r = s; } break;
        case INTEGER: { *r        = to_string(*(static_cast<const int *>(from))); } break;
        default:      { result    = false; } break;
    }
    return result;
}

bool AMetaType::toList(void *to, const void *from, const uint32_t fromType) {
    PROFILE_FUNCTION()
    bool result = true;
    AVariantList *r = static_cast<AVariantList *>(to);
    switch(fromType) {
        case VECTOR2: {
            const Vector2 v = *(reinterpret_cast<const Vector2 *>(from));
            for(int i = 0; i < 2; i++) {
                r->push_back(v[i]);
            }
        } break;
        case VECTOR3: {
            const Vector3 v = *(reinterpret_cast<const Vector3 *>(from));
            for(int i = 0; i < 3; i++) {
                r->push_back(v[i]);
            }
        } break;
        case VECTOR4: {
            const Vector4 v = *(reinterpret_cast<const Vector4 *>(from));
            for(int i = 0; i < 4; i++) {
                r->push_back(v[i]);
            }
        } break;
        case QUATERNION: {
            const Quaternion v = *(reinterpret_cast<const Quaternion *>(from));
            for(int i = 0; i < 4; i++) {
                r->push_back(v[i]);
            }
        } break;
        case MATRIX3: {
            const Matrix3 v = *(reinterpret_cast<const Matrix3 *>(from));
            for(int i = 0; i < 9; i++) {
                r->push_back(v[i]);
            }
        } break;
        case MATRIX4:{
            const Matrix4 v = *(reinterpret_cast<const Matrix4 *>(from));
            for(int i = 0; i < 16; i++) {
                r->push_back(v[i]);
            }
        } break;
        default:    { result    = false; } break;
    }
    return result;
}

bool AMetaType::toVector2(void *to, const void *from, const uint32_t fromType) {
    PROFILE_FUNCTION()
    bool result    = true;
    Vector2 *r    = static_cast<Vector2 *>(to);
    switch(fromType) {
        case INTEGER: { *r  = Vector2(*(static_cast<const int *>(from))); } break;
        case DOUBLE:  { *r  = Vector2(*(static_cast<const double *>(from))); } break;
        case VARIANTLIST: {
            const AVariantList *list    = reinterpret_cast<const AVariantList *>(from);
            auto it = list->begin();
            for(int i = 0; i < 2; i++, it++) {
                (*r)[i] = (*it).toDouble();
            }
        } break;
        default:    { result    = false; } break;
    }
    return result;
}

bool AMetaType::toVector3(void *to, const void *from, const uint32_t fromType) {
    PROFILE_FUNCTION()
    bool result     = true;
    Vector3 *r    = static_cast<Vector3 *>(to);
    switch(fromType) {
        case INTEGER: { *r  = Vector3(*(static_cast<const int *>(from))); } break;
        case DOUBLE:  { *r  = Vector3(*(static_cast<const double *>(from))); } break;
        case VARIANTLIST: {
            const AVariantList *list    = reinterpret_cast<const AVariantList *>(from);
            auto it = list->begin();
            for(int i = 0; i < 3; i++, it++) {
                (*r)[i] = (*it).toDouble();
            }
        } break;
        case VECTOR2:  { *r = Vector3(*(static_cast<const Vector2 *>(from)), 0.0); } break;
        default:    { result    = false; } break;
    }
    return result;
}

bool AMetaType::toVector4(void *to, const void *from, const uint32_t fromType) {
    PROFILE_FUNCTION()
    bool result     = true;
    Vector4 *r    = static_cast<Vector4 *>(to);
    switch(fromType) {
        case INTEGER:{ *r  = Vector4(*(static_cast<const int *>(from))); } break;
        case DOUBLE: { *r  = Vector4(*(static_cast<const double *>(from))); } break;
        case VARIANTLIST: {
            const AVariantList *list    = reinterpret_cast<const AVariantList *>(from);
            auto it = list->begin();
            for(int i = 0; i < 4; i++, it++) {
                (*r)[i] = (*it).toDouble();
            }
        } break;
        case VECTOR2:   { *r = Vector4(*(static_cast<const Vector2 *>(from)), 0.0, 1.0); } break;
        case VECTOR3:   { *r = Vector4(*(static_cast<const Vector3 *>(from)), 1.0); } break;
        default:    { result    = false; } break;
    }
    return result;
}

bool AMetaType::toMatrix3(void *to, const void *from, const uint32_t fromType) {
    PROFILE_FUNCTION()
    bool result     = true;
    Matrix3 *r    = static_cast<Matrix3 *>(to);
    switch(fromType) {
        case VARIANTLIST: {
            const AVariantList *list    = reinterpret_cast<const AVariantList *>(from);
            auto it = list->begin();
            for(int i = 0; i < 9; i++, it++) {
                (*r)[i]   = (*it).toDouble();
            }
        } break;
        default: { result   = false; } break;
    }
    return result;
}

bool AMetaType::toMatrix4(void *to, const void *from, const uint32_t fromType) {
    PROFILE_FUNCTION()
    bool result     = true;
    Matrix4 *r    = static_cast<Matrix4 *>(to);
    switch(fromType) {
        case VARIANTLIST: {
            const AVariantList *list    = reinterpret_cast<const AVariantList *>(from);
            auto it = list->begin();
            for(int i = 0; i < 16; i++, it++) {
                (*r)[i] = (*it).toDouble();
            }
        } break;
        default: { result   = false; } break;
    }
    return result;
}

bool AMetaType::toQuaternion(void *to, const void *from, const uint32_t fromType) {
    PROFILE_FUNCTION()
    bool result     = true;
    Quaternion *r  = static_cast<Quaternion *>(to);
    switch(fromType) {
        case VARIANTLIST: {
            const AVariantList *list  = reinterpret_cast<const AVariantList *>(from);
            auto it = list->begin();
            for(int i = 0; i < 4; i++, it++) {
                (*r)[i] = (*it).toDouble();
            }
        } break;
        case VECTOR3: { Matrix3 m; m.rotate(*(static_cast<const Vector3 *>(from))); *r = Quaternion(m); } break;
        default:    { result    = false; } break;
    }
    return result;
}
