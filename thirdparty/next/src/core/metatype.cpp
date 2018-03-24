#include "core/metatype.h"

#include <list>

#include "math/amath.h"
#include "core/variant.h"



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

typedef map<uint32_t, MetaType::Table> TypeMap;
typedef map<string, uint32_t>           NameMap;
typedef map<uint32_t, map<uint32_t, MetaType::converterCallback> > ConverterMap;

uint32_t MetaType::s_NextId = MetaType::USERTYPE;
static TypeMap s_Types = {
    {MetaType::BOOLEAN,     DECLARE_BUILT_TYPE(bool)},
    {MetaType::INTEGER,     DECLARE_BUILT_TYPE(int)},
    {MetaType::FLOAT,       DECLARE_BUILT_TYPE(float)},
    {MetaType::STRING,      DECLARE_BUILT_TYPE(string)},
    {MetaType::VARIANTMAP,  DECLARE_BUILT_TYPE(VariantMap)},
    {MetaType::VARIANTLIST, DECLARE_BUILT_TYPE(VariantList)},
    {MetaType::BYTEARRAY,   DECLARE_BUILT_TYPE(ByteArray)},
    {MetaType::VECTOR2,     DECLARE_BUILT_TYPE(Vector2)},
    {MetaType::VECTOR3,     DECLARE_BUILT_TYPE(Vector3)},
    {MetaType::VECTOR4,     DECLARE_BUILT_TYPE(Vector4)},
    {MetaType::QUATERNION,  DECLARE_BUILT_TYPE(Quaternion)},
    {MetaType::MATRIX3,     DECLARE_BUILT_TYPE(Matrix3)},
    {MetaType::MATRIX4,     DECLARE_BUILT_TYPE(Matrix4)}
};

static ConverterMap s_Converters= {
    {MetaType::BOOLEAN,   {{MetaType::INTEGER,  &MetaType::toBoolean},
                           {MetaType::FLOAT,    &MetaType::toBoolean},
                           {MetaType::STRING,   &MetaType::toBoolean}}},

    {MetaType::INTEGER,   {{MetaType::BOOLEAN,  &MetaType::toInteger},
                           {MetaType::FLOAT,    &MetaType::toInteger},
                           {MetaType::STRING,   &MetaType::toInteger}}},

    {MetaType::FLOAT,     {{MetaType::BOOLEAN,  &MetaType::toFloat},
                           {MetaType::INTEGER,  &MetaType::toFloat},
                           {MetaType::STRING,   &MetaType::toFloat}}},

    {MetaType::STRING,    {{MetaType::BOOLEAN,  &MetaType::toString},
                           {MetaType::INTEGER,  &MetaType::toString},
                           {MetaType::FLOAT,    &MetaType::toString}}},

    {MetaType::VARIANTLIST,{{MetaType::VECTOR2,     &MetaType::toList},
                            {MetaType::VECTOR3,     &MetaType::toList},
                            {MetaType::VECTOR4,     &MetaType::toList},
                            {MetaType::QUATERNION,  &MetaType::toList},
                            {MetaType::MATRIX3,     &MetaType::toList},
                            {MetaType::MATRIX4,     &MetaType::toList}}},

    {MetaType::VECTOR2,    {{MetaType::INTEGER,     &MetaType::toVector2},
                            {MetaType::FLOAT,       &MetaType::toVector2},
                            {MetaType::VARIANTLIST, &MetaType::toVector2}}},

    {MetaType::VECTOR3,    {{MetaType::INTEGER,     &MetaType::toVector3},
                            {MetaType::FLOAT,       &MetaType::toVector3},
                            {MetaType::VARIANTLIST, &MetaType::toVector3},
                            {MetaType::VECTOR2,     &MetaType::toVector3}}},

    {MetaType::VECTOR4,    {{MetaType::INTEGER,     &MetaType::toVector4},
                            {MetaType::FLOAT,       &MetaType::toVector4},
                            {MetaType::VARIANTLIST, &MetaType::toVector4},
                            {MetaType::VECTOR2,     &MetaType::toVector4},
                            {MetaType::MATRIX3,     &MetaType::toVector4}}},

    {MetaType::QUATERNION, {{MetaType::VARIANTLIST, &MetaType::toQuaternion},
                            {MetaType::MATRIX3,     &MetaType::toQuaternion}}},

    {MetaType::MATRIX3,    {{MetaType::VARIANTLIST,&MetaType::toMatrix3}}},

    {MetaType::MATRIX4,    {{MetaType::VARIANTLIST,&MetaType::toMatrix4}}}
};

static NameMap s_Names = {
    {"bool",            MetaType::BOOLEAN},
    {"int",             MetaType::INTEGER},
    {"float",           MetaType::FLOAT},
    {"string",          MetaType::STRING},
    {"map",             MetaType::VARIANTMAP},
    {"list",            MetaType::VARIANTLIST},
    {"AByteArray",      MetaType::BYTEARRAY},
    {"Vector2",         MetaType::VECTOR2},
    {"Vector3",         MetaType::VECTOR3},
    {"Vector4",         MetaType::VECTOR4},
    {"Quaternion",      MetaType::QUATERNION},
    {"Matrix3",         MetaType::MATRIX3},
    {"Matrix4",         MetaType::MATRIX4}
};

MetaType::MetaType(const Table *table) :
        m_pTable(table) {
    PROFILE_FUNCTION()
}

const char *MetaType::name() const {
    PROFILE_FUNCTION()
    return m_pTable->name;
}

int MetaType::size() const {
    PROFILE_FUNCTION()
    return m_pTable->get_size();
}

void *MetaType::construct(void *where, const void *copy) const {
    PROFILE_FUNCTION()
    if(copy) {
        m_pTable->clone(&copy, &where);
        return where;
    } else {
        m_pTable->construct(&where);
        return where;
    }
}

void *MetaType::create(const void *copy) const {
    PROFILE_FUNCTION()
    void *where = nullptr;
    m_pTable->static_new(&where);

    if(copy) {
        m_pTable->clone(&copy, &where);
    }
    return where;
}

void MetaType::destroy(void *data) const {
    PROFILE_FUNCTION()
    m_pTable->static_delete(&data);
}

void MetaType::destruct(void *data) const {
    PROFILE_FUNCTION()
    m_pTable->destruct(&data);
}

bool MetaType::compare(const void *left, const void *right) const {
    PROFILE_FUNCTION()
    return m_pTable->compare(&left, &right);
}

bool MetaType::isValid() const {
    PROFILE_FUNCTION()
    return (m_pTable != nullptr);
}

uint32_t MetaType::registerType(Table &table) {
    PROFILE_FUNCTION()
    uint32_t result = ++MetaType::s_NextId;
    s_Types[result] = table;
    s_Names[table.name] = result;
    return result;
}

uint32_t MetaType::type(const char *name) {
    PROFILE_FUNCTION()
    auto it = s_Names.find(name);
    if(it != s_Names.end()) {
        return it->second;
    }
    return INVALID;
}
uint32_t MetaType::type(const type_info &type) {
    PROFILE_FUNCTION()
    for(auto it : s_Types) {
        if(it.second.index() == type_index(type) ) {
            return it.first;
        }
    }
    return INVALID;
}

const char *MetaType::name(uint32_t type) {
    PROFILE_FUNCTION()
    auto it = s_Types.find(type);
    if(it != s_Types.end()) {
        return MetaType(&(it->second)).name();
    }
    return nullptr;
}

int MetaType::size(uint32_t type) {
    PROFILE_FUNCTION()
    auto it = s_Types.find(type);
    if(it != s_Types.end()) {
        return MetaType(&(it->second)).size();
    }
    return 0;
}

void *MetaType::construct(uint32_t type, void *where, const void *copy) {
    PROFILE_FUNCTION()
    auto it = s_Types.find(type);
    if(it != s_Types.end()) {
        return MetaType(&(it->second)).construct(where, copy);
    }
    return nullptr;
}

void *MetaType::create(uint32_t type, const void *copy) {
    PROFILE_FUNCTION()
    auto it = s_Types.find(type);
    if(it != s_Types.end()) {
        return MetaType(&(it->second)).create(copy);
    }
    return nullptr;
}

void MetaType::destroy(uint32_t type, void *data) {
    PROFILE_FUNCTION()
    auto it = s_Types.find(type);
    if(it != s_Types.end()) {
        MetaType(&(it->second)).destroy(data);
    }
}

void MetaType::destruct(uint32_t type, void *data) {
    PROFILE_FUNCTION()
    auto it = s_Types.find(type);
    if(it != s_Types.end()) {
        MetaType(&(it->second)).destruct(data);
    }
}

bool MetaType::compare(const void *left, const void *right, uint32_t type) {
    PROFILE_FUNCTION()
    auto it = s_Types.find(type);
    if(it != s_Types.end()) {
        return MetaType(&(it->second)).compare(left, right);
    }
    return false;
}

bool MetaType::convert(const void *from, uint32_t fromType, void *to, uint32_t toType) {
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

bool MetaType::registerConverter(uint32_t from, uint32_t to, converterCallback function) {
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

bool MetaType::hasConverter(uint32_t from, uint32_t to) {
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

bool MetaType::toBoolean(void *to, const void *from, const uint32_t fromType) {
    PROFILE_FUNCTION()
    bool result = true;
    bool *r     = static_cast<bool *>(to);
    switch(fromType) {
        case INTEGER: { *r  = *(static_cast<const int *>(from)) != 0; } break;
        case FLOAT:   { *r  = *(static_cast<const float *>(from)) != 0; } break;
        case STRING:  {
            string s  = *(static_cast<const string *>(from));
            *r = (s != "false" || s != "0" || !s.empty());
        }  break;
        default:      { result  = false; } break;
    }
    return result;
}

bool MetaType::toInteger(void *to, const void *from, const uint32_t fromType) {
    PROFILE_FUNCTION()
    bool result = true;
    int *r      = static_cast<int *>(to);
    switch(fromType) {
        case BOOLEAN: { *r      = (*(static_cast<const bool *>(from))) ? 1 : 0; } break;
        case FLOAT: { double f  = *(static_cast<const float *>(from)); *r = int(f); f -= *r; *r += (f >= 0.5f) ? 1 : 0; } break;
        case STRING: {
            string s  = *(static_cast<const string *>(from));
            char *end;
            *r        = strtol(s.c_str(), &end, 10);
        } break;
        default:    { result    = false; } break;
    }
    return result;
}

bool MetaType::toFloat(void *to, const void *from, const uint32_t fromType) {
    PROFILE_FUNCTION()
    bool result = true;
    float *r    = static_cast<float *>(to);
    switch(fromType) {
        case BOOLEAN: { *r  = areal(*(static_cast<const bool *>(from))); } break;
        case INTEGER: { *r  = areal(*(static_cast<const int *>(from))); } break;
        case STRING:  {
            string s    = *(static_cast<const string *>(from));
            char *end;
            *r          = strtof(s.c_str(), &end);
        } break;
        default:      { result  = false; } break;
    }
    return result;
}

bool MetaType::toString(void *to, const void *from, const uint32_t fromType) {
    PROFILE_FUNCTION()
    bool result = true;
    string *r   = static_cast<string *>(to);
    switch(fromType) {
        case BOOLEAN: { *r        = (*(static_cast<const bool *>(from))) ? "true" : "false"; } break;
        case FLOAT:   { string s  = to_string(*(static_cast<const float *>(from))); *r = s; } break;
        case INTEGER: { *r        = to_string(*(static_cast<const int *>(from))); } break;
        default:      { result    = false; } break;
    }
    return result;
}

bool MetaType::toList(void *to, const void *from, const uint32_t fromType) {
    PROFILE_FUNCTION()
    bool result = true;
    VariantList *r = static_cast<VariantList *>(to);
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

bool MetaType::toVector2(void *to, const void *from, const uint32_t fromType) {
    PROFILE_FUNCTION()
    bool result    = true;
    Vector2 *r    = static_cast<Vector2 *>(to);
    switch(fromType) {
        case INTEGER:   { *r  = Vector2(areal(*(static_cast<const int *>(from)))); } break;
        case FLOAT:     { *r  = Vector2(*(static_cast<const float *>(from))); } break;
        case VARIANTLIST: {
            const VariantList *list    = reinterpret_cast<const VariantList *>(from);
            auto it = list->begin();
            for(int i = 0; i < 2; i++, it++) {
                (*r)[i] = (*it).toFloat();
            }
        } break;
        default:    { result    = false; } break;
    }
    return result;
}

bool MetaType::toVector3(void *to, const void *from, const uint32_t fromType) {
    PROFILE_FUNCTION()
    bool result     = true;
    Vector3 *r    = static_cast<Vector3 *>(to);
    switch(fromType) {
        case INTEGER:   { *r  = Vector3(areal(*(static_cast<const int *>(from)))); } break;
        case FLOAT:     { *r  = Vector3(*(static_cast<const float *>(from))); } break;
        case VARIANTLIST: {
            const VariantList *list    = reinterpret_cast<const VariantList *>(from);
            auto it = list->begin();
            for(int i = 0; i < 3; i++, it++) {
                (*r)[i] = (*it).toFloat();
            }
        } break;
        case VECTOR2:  { *r = Vector3(*(static_cast<const Vector2 *>(from)), 0.0); } break;
        default:    { result    = false; } break;
    }
    return result;
}

bool MetaType::toVector4(void *to, const void *from, const uint32_t fromType) {
    PROFILE_FUNCTION()
    bool result     = true;
    Vector4 *r    = static_cast<Vector4 *>(to);
    switch(fromType) {
        case INTEGER:   { *r  = Vector4(areal(*(static_cast<const int *>(from)))); } break;
        case FLOAT:     { *r  = Vector4(*(static_cast<const float *>(from))); } break;
        case VARIANTLIST: {
            const VariantList *list    = reinterpret_cast<const VariantList *>(from);
            auto it = list->begin();
            for(int i = 0; i < 4; i++, it++) {
                (*r)[i] = (*it).toFloat();
            }
        } break;
        case VECTOR2:   { *r = Vector4(*(static_cast<const Vector2 *>(from)), 0.0, 1.0); } break;
        case VECTOR3:   { *r = Vector4(*(static_cast<const Vector3 *>(from)), 1.0); } break;
        default:    { result    = false; } break;
    }
    return result;
}

bool MetaType::toMatrix3(void *to, const void *from, const uint32_t fromType) {
    PROFILE_FUNCTION()
    bool result = true;
    Matrix3 *r  = static_cast<Matrix3 *>(to);
    switch(fromType) {
        case VARIANTLIST: {
            const VariantList *list    = reinterpret_cast<const VariantList *>(from);
            auto it = list->begin();
            for(int i = 0; i < 9; i++, it++) {
                (*r)[i]   = (*it).toFloat();
            }
        } break;
        default: { result   = false; } break;
    }
    return result;
}

bool MetaType::toMatrix4(void *to, const void *from, const uint32_t fromType) {
    PROFILE_FUNCTION()
    bool result = true;
    Matrix4 *r  = static_cast<Matrix4 *>(to);
    switch(fromType) {
        case VARIANTLIST: {
            const VariantList *list    = reinterpret_cast<const VariantList *>(from);
            auto it = list->begin();
            for(int i = 0; i < 16; i++, it++) {
                (*r)[i] = (*it).toFloat();
            }
        } break;
        default: { result   = false; } break;
    }
    return result;
}

bool MetaType::toQuaternion(void *to, const void *from, const uint32_t fromType) {
    PROFILE_FUNCTION()
    bool result     = true;
    Quaternion *r   = static_cast<Quaternion *>(to);
    switch(fromType) {
        case VARIANTLIST: {
            const VariantList *list  = reinterpret_cast<const VariantList *>(from);
            auto it = list->begin();
            for(int i = 0; i < 4; i++, it++) {
                (*r)[i] = (*it).toFloat();
            }
        } break;
        case VECTOR3:   { Matrix3 m; m.rotate(*(static_cast<const Vector3 *>(from))); *r = Quaternion(m); } break;
        default:        { result    = false; } break;
    }
    return result;
}
