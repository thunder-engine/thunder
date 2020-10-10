#include "core/metatype.h"

#include <list>

#include "math/amath.h"
#include "core/variant.h"

/*!
    \fn uint32_t registerMetaType(const char *typeName)

    Registers a new type with type T and \a typeName as MetaType.
    After registration it can be used as Variant in MetaObject system.
*/
/*!
    \fn uint32_t MetaType::type()

    Returns the type ID for type T.
*/
#define DECLARE_BUILT_TYPE(TYPE) \
    { \
        nullptr, \
        nullptr, \
        nullptr,\
        TypeFuncs<TYPE>::size, \
        TypeFuncs<TYPE>::static_new, \
        TypeFuncs<TYPE>::construct, \
        TypeFuncs<TYPE>::static_delete, \
        TypeFuncs<TYPE>::destruct, \
        TypeFuncs<TYPE>::clone, \
        TypeFuncs<TYPE>::compare, \
        TypeFuncs<TYPE>::index, \
        #TYPE, \
        false \
    }

typedef map<string, uint32_t>           NameMap;
typedef map<uint32_t, map<uint32_t, MetaType::converterCallback> > ConverterMap;

bool toBoolean(void *to, const void *from, const uint32_t fromType) {
    PROFILE_FUNCTION();
    bool result = true;
    bool *r     = static_cast<bool *>(to);
    switch(fromType) {
        case MetaType::INTEGER: { *r  = *(static_cast<const int *>(from)) != 0; } break;
        case MetaType::FLOAT:   { *r  = *(static_cast<const float *>(from)) != 0; } break;
        case MetaType::STRING:  {
            string s  = *(static_cast<const string *>(from));
            *r = (s != "false" || s != "0" || !s.empty());
        }  break;
        default:      { result  = false; } break;
    }
    return result;
}

bool toInteger(void *to, const void *from, const uint32_t fromType) {
    PROFILE_FUNCTION();
    bool result = true;
    int *r      = static_cast<int *>(to);
    switch(fromType) {
        case MetaType::BOOLEAN: { *r      = (*(static_cast<const bool *>(from))) ? 1 : 0; } break;
        case MetaType::FLOAT:   { double f  = *(static_cast<const float *>(from)); *r = int(f); f -= *r; *r += (f >= 0.5f) ? 1 : 0; } break;
        case MetaType::STRING:  {
            string s  = *(static_cast<const string *>(from));
            char *end;
            *r        = strtol(s.c_str(), &end, 10);
        } break;
        default:    { result    = false; } break;
    }
    return result;
}

bool toFloat(void *to, const void *from, const uint32_t fromType) {
    PROFILE_FUNCTION();
    bool result = true;
    float *r    = static_cast<float *>(to);
    switch(fromType) {
        case MetaType::BOOLEAN: { *r  = areal(*(static_cast<const bool *>(from))); } break;
        case MetaType::INTEGER: { *r  = areal(*(static_cast<const int *>(from))); } break;
        case MetaType::STRING:  {
            string s    = *(static_cast<const string *>(from));
            char *end;
            *r          = strtof(s.c_str(), &end);
        } break;
        default:      { result  = false; } break;
    }
    return result;
}

bool toString(void *to, const void *from, const uint32_t fromType) {
    PROFILE_FUNCTION();
    bool result = true;
    string *r   = static_cast<string *>(to);
    switch(fromType) {
        case MetaType::BOOLEAN: { *r        = (*(static_cast<const bool *>(from))) ? "true" : "false"; } break;
        case MetaType::FLOAT:   { string s  = to_string(*(static_cast<const float *>(from))); *r = s; } break;
        case MetaType::INTEGER: { *r        = to_string(*(static_cast<const int *>(from))); } break;
        default:      { result    = false; } break;
    }
    return result;
}

bool toList(void *to, const void *from, const uint32_t fromType) {
    PROFILE_FUNCTION();
    bool result = true;
    VariantList *r = static_cast<VariantList *>(to);
    switch(fromType) {
        case MetaType::VECTOR2: {
            const Vector2 v = *(reinterpret_cast<const Vector2 *>(from));
            for(int i = 0; i < 2; i++) {
                r->push_back(v[i]);
            }
        } break;
        case MetaType::VECTOR3: {
            const Vector3 v = *(reinterpret_cast<const Vector3 *>(from));
            for(int i = 0; i < 3; i++) {
                r->push_back(v[i]);
            }
        } break;
        case MetaType::VECTOR4: {
            const Vector4 v = *(reinterpret_cast<const Vector4 *>(from));
            for(int i = 0; i < 4; i++) {
                r->push_back(v[i]);
            }
        } break;
        case MetaType::QUATERNION: {
            const Quaternion v = *(reinterpret_cast<const Quaternion *>(from));
            for(int i = 0; i < 4; i++) {
                r->push_back(v[i]);
            }
        } break;
        case MetaType::MATRIX3: {
            const Matrix3 v = *(reinterpret_cast<const Matrix3 *>(from));
            for(int i = 0; i < 9; i++) {
                r->push_back(v[i]);
            }
        } break;
        case MetaType::MATRIX4:{
            const Matrix4 v = *(reinterpret_cast<const Matrix4 *>(from));
            for(int i = 0; i < 16; i++) {
                r->push_back(v[i]);
            }
        } break;
        default:    { result    = false; } break;
    }
    return result;
}

bool toVector2(void *to, const void *from, const uint32_t fromType) {
    PROFILE_FUNCTION();
    bool result    = true;
    Vector2 *r    = static_cast<Vector2 *>(to);
    switch(fromType) {
        case MetaType::INTEGER:   { *r  = Vector2(areal(*(static_cast<const int *>(from)))); } break;
        case MetaType::FLOAT:     { *r  = Vector2(*(static_cast<const float *>(from))); } break;
        case MetaType::VARIANTLIST: {
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

bool toVector3(void *to, const void *from, const uint32_t fromType) {
    PROFILE_FUNCTION();
    bool result     = true;
    Vector3 *r    = static_cast<Vector3 *>(to);
    switch(fromType) {
        case MetaType::INTEGER: { *r  = Vector3(areal(*(static_cast<const int *>(from)))); } break;
        case MetaType::FLOAT:   { *r  = Vector3(*(static_cast<const float *>(from))); } break;
        case MetaType::VARIANTLIST: {
            const VariantList *list    = reinterpret_cast<const VariantList *>(from);
            auto it = list->begin();
            for(int i = 0; i < 3; i++, it++) {
                (*r)[i] = (*it).toFloat();
            }
        } break;
        case MetaType::VECTOR2: { *r = Vector3(*(static_cast<const Vector2 *>(from)), 0.0); } break;
        default:    { result    = false; } break;
    }
    return result;
}

bool toVector4(void *to, const void *from, const uint32_t fromType) {
    PROFILE_FUNCTION();
    bool result     = true;
    Vector4 *r    = static_cast<Vector4 *>(to);
    switch(fromType) {
        case MetaType::INTEGER:   { *r  = Vector4(areal(*(static_cast<const int *>(from)))); } break;
        case MetaType::FLOAT:     { *r  = Vector4(*(static_cast<const float *>(from))); } break;
        case MetaType::VARIANTLIST: {
            const VariantList *list    = reinterpret_cast<const VariantList *>(from);
            auto it = list->begin();
            for(int i = 0; i < 4; i++, it++) {
                (*r)[i] = (*it).toFloat();
            }
        } break;
        case MetaType::VECTOR2:   { *r = Vector4(*(static_cast<const Vector2 *>(from)), 0.0, 1.0); } break;
        case MetaType::VECTOR3:   { *r = Vector4(*(static_cast<const Vector3 *>(from)), 1.0); } break;
        default:    { result    = false; } break;
    }
    return result;
}

bool toMatrix3(void *to, const void *from, const uint32_t fromType) {
    PROFILE_FUNCTION();
    bool result = true;
    Matrix3 *r  = static_cast<Matrix3 *>(to);
    switch(fromType) {
        case MetaType::VARIANTLIST: {
            const VariantList *list    = reinterpret_cast<const VariantList *>(from);
            auto it = list->begin();
            for(int i = 0; i < 9; i++, it++) {
                (*r)[i] = (*it).toFloat();
            }
        } break;
        default: { result   = false; } break;
    }
    return result;
}

bool toMatrix4(void *to, const void *from, const uint32_t fromType) {
    PROFILE_FUNCTION();
    bool result = true;
    Matrix4 *r  = static_cast<Matrix4 *>(to);
    switch(fromType) {
        case MetaType::VARIANTLIST: {
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

bool toQuaternion(void *to, const void *from, const uint32_t fromType) {
    PROFILE_FUNCTION();
    bool result     = true;
    Quaternion *r   = static_cast<Quaternion *>(to);
    switch(fromType) {
        case MetaType::VARIANTLIST: {
            const VariantList *list  = reinterpret_cast<const VariantList *>(from);
            auto it = list->begin();
            for(int i = 0; i < 4; i++, it++) {
                (*r)[i] = (*it).toFloat();
            }
        } break;
        case MetaType::VECTOR3: {
            Matrix3 m;
            m.rotate(*(static_cast<const Vector3 *>(from)));
            *r = Quaternion(m);
        } break;
        default:        { result    = false; } break;
    }
    return result;
}

uint32_t MetaType::s_NextId = MetaType::USERTYPE;
static MetaType::TypeMap s_Types = {
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
    {MetaType::MATRIX4,     DECLARE_BUILT_TYPE(Matrix4)},
    {MetaType::RAY,         DECLARE_BUILT_TYPE(Ray)}
};

static ConverterMap s_Converters= {
    {MetaType::BOOLEAN,    {{MetaType::INTEGER,     &toBoolean},
                            {MetaType::FLOAT,       &toBoolean},
                            {MetaType::STRING,      &toBoolean}}},

    {MetaType::INTEGER,    {{MetaType::BOOLEAN,     &toInteger},
                            {MetaType::FLOAT,       &toInteger},
                            {MetaType::STRING,      &toInteger}}},

    {MetaType::FLOAT,      {{MetaType::BOOLEAN,     &toFloat},
                            {MetaType::INTEGER,     &toFloat},
                            {MetaType::STRING,      &toFloat}}},

    {MetaType::STRING,     {{MetaType::BOOLEAN,     &toString},
                            {MetaType::INTEGER,     &toString},
                            {MetaType::FLOAT,       &toString}}},

    {MetaType::VARIANTLIST,{{MetaType::VECTOR2,     &toList},
                            {MetaType::VECTOR3,     &toList},
                            {MetaType::VECTOR4,     &toList},
                            {MetaType::QUATERNION,  &toList},
                            {MetaType::MATRIX3,     &toList},
                            {MetaType::MATRIX4,     &toList}}},

    {MetaType::VECTOR2,    {{MetaType::INTEGER,     &toVector2},
                            {MetaType::FLOAT,       &toVector2},
                            {MetaType::VARIANTLIST, &toVector2}}},

    {MetaType::VECTOR3,    {{MetaType::INTEGER,     &toVector3},
                            {MetaType::FLOAT,       &toVector3},
                            {MetaType::VARIANTLIST, &toVector3},
                            {MetaType::VECTOR2,     &toVector3}}},

    {MetaType::VECTOR4,    {{MetaType::INTEGER,     &toVector4},
                            {MetaType::FLOAT,       &toVector4},
                            {MetaType::VARIANTLIST, &toVector4},
                            {MetaType::VECTOR2,     &toVector4},
                            {MetaType::MATRIX3,     &toVector4}}},

    {MetaType::QUATERNION, {{MetaType::VARIANTLIST, &toQuaternion},
                            {MetaType::MATRIX3,     &toQuaternion}}},

    {MetaType::MATRIX3,    {{MetaType::VARIANTLIST, &toMatrix3}}},

    {MetaType::MATRIX4,    {{MetaType::VARIANTLIST, &toMatrix4}}}
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
/*!
    \class MetaType
    \brief The MetaType provides an interface to retrieve information about data type at runtime.
    \since Next 1.0
    \inmodule Core

    This class is designed for retrieving of runtime type information with additional functionality.

    Some of registered types can be automatically converted to different types with MetaType::convert functunction.
    The following conversions are predefined:
    \table
    \header
        \li Type
        \li Convert to
    \row
        \li MetaType::BOOLEAN
        \li MetaType::INTEGER, MetaType::FLOAT, MetaType::STRING
    \row
        \li MetaType::INTEGER
        \li MetaType::BOOLEAN, MetaType::FLOAT, MetaType::STRING, MetaType::VECTOR2, MetaType::VECTOR3, MetaType::VECTOR4
    \row
        \li MetaType::FLOAT
        \li MetaType::BOOLEAN, MetaType::INTEGER, MetaType::STRING, MetaType::VECTOR2, MetaType::VECTOR3, MetaType::VECTOR4
    \row
        \li MetaType::STRING
        \li MetaType::BOOLEAN, MetaType::INTEGER, MetaType::FLOAT
    \row
        \li MetaType::VARIANTMAP
        \li
    \row
        \li MetaType::VARIANTLIST
        \li MetaType::VECTOR2, MetaType::VECTOR3, MetaType::VECTOR4, MetaType::MATRIX3, MetaType::MATRIX4, MetaType::QUATERNION
    \row
        \li MetaType::VECTOR2
        \li MetaType::VARIANTLIST, MetaType::VECTOR3, MetaType::VECTOR4
    \row
        \li MetaType::VECTOR3
        \li MetaType::VARIANTLIST, MetaType::VECTOR4
    \row
        \li MetaType::VECTOR4
        \li MetaType::VARIANTLIST
    \row
        \li MetaType::QUATERNION
        \li MetaType::VARIANTLIST
    \row
        \li MetaType::MATRIX3
        \li MetaType::VARIANTLIST
    \row
        \li MetaType::MATRIX4
        \li MetaType::VARIANTLIST
    \endtable

    To convert values to other types developer should define own conversion type function using MetaType::registerConverter() function
*/
/*!
    \typedef MetaType::converterCallback

    Callback which contain address to converter function.
    This converter must be able to convert \a from value with \a fromType type to \a to value with type represented by this MetaType.
*/
/*!
    Constructs MetaType object wich will contain information provided in a \a table.
*/
MetaType::MetaType(const Table *table) :
        m_pTable(table) {
    PROFILE_FUNCTION();
}
/*!
    Returns the name of type.
*/
const char *MetaType::name() const {
    PROFILE_FUNCTION();
    return m_pTable->name;
}
/*!
    Returns the size of type.
*/
int MetaType::size() const {
    PROFILE_FUNCTION();
    return m_pTable->get_size();
}
/*!
    Constructs a value of the given type, wich represented by current MetaType object in the existing memory addressed by \a where, that is a copy of \a copy, and returns where.
    If \a copy is zero, the value is default constructed.
*/
void *MetaType::construct(void *where, const void *copy) const {
    PROFILE_FUNCTION();
    if(copy) {
        m_pTable->clone(&copy, &where);
    } else {
        m_pTable->construct(where);
    }
    return where;
}
/*!
    Returns a copy of \a copy value, with type, wich represented by current MetaType object.
    If \a copy is null, creates a default constructed instance.
*/
void *MetaType::create(const void *copy) const {
    PROFILE_FUNCTION();
    void *where = nullptr;
    if(copy) {
        m_pTable->clone(&copy, &where);
    } else {
        where   = m_pTable->static_new();
    }
    return where;
}
/*!
    Destroys the value with type, wich represented by current MetaType object, located at \a data.
    This function calls delete operator.
*/
void MetaType::destroy(void *data) const {
    PROFILE_FUNCTION();
    m_pTable->static_delete(&data);
}
/*!
    Destructs the value with type, wich represented by current MetaType object, located at \a data.
    Unlike destroy(), this function only invokes the type's destructor, it doesn't invoke the delete operator.
*/
void MetaType::destruct(void *data) const {
    PROFILE_FUNCTION();
    m_pTable->destruct(data);
}
/*!
    Returns true in case of \a left value is equal to \a right value; otherwise returns false.
*/
bool MetaType::compare(const void *left, const void *right) const {
    PROFILE_FUNCTION();
    return m_pTable->compare(&left, &right);
}
/*!
    Returns true in case of this MetaType object contain valid information; otherwise returns false.
*/
bool MetaType::isValid() const {
    PROFILE_FUNCTION();
    return (m_pTable != nullptr);
}
/*!
    Returns flags for the type.
*/
int MetaType::flags() const {
    return m_pTable->flags;
}
/*!
    Registers type by type MetaType::Table \a table. Use registerMetaType() instead this function.
    Returns an ID of registered type.
*/
uint32_t MetaType::registerType(Table &table) {
    PROFILE_FUNCTION();
    uint32_t result = ++MetaType::s_NextId;
    s_Types[result] = table;
    s_Names[table.name] = result;
    return result;
}
/*!
    Unregisters type by type MetaType::Table \a table. Use unregisterMetaType() instead this function.
*/
void MetaType::unregisterType(Table &table) {
    PROFILE_FUNCTION();
    auto it = s_Names.find(table.name);
    if(it != s_Names.end()) {
        uint32_t id = it->second;
        auto name = s_Types.find(id);
        if(name != s_Types.end()) {
            s_Types.erase(name);
        }
        s_Names.erase(it);
    }
}
/*!
    Returns an ID of type with type \a name.
    Returns MetaType::INVALID for unregistered type.
*/
uint32_t MetaType::type(const char *name) {
    PROFILE_FUNCTION();
    auto it = s_Names.find(name);
    if(it != s_Names.end()) {
        return it->second;
    }
    return INVALID;
}
/*!
    Returns an ID of type with \a type info.
    Returns MetaType::INVALID for unregistered \a type.
*/
uint32_t MetaType::type(const type_info &type) {
    PROFILE_FUNCTION();
    for(auto it : s_Types) {
        if(it.second.index() == type_index(type) ) {
            return it.first;
        }
    }
    return INVALID;
}
/*!
    Returns a name of type with \a type ID.
    Returns nullptr for unregistered \a type.
*/
const char *MetaType::name(uint32_t type) {
    PROFILE_FUNCTION();
    auto it = s_Types.find(type);
    if(it != s_Types.end()) {
        return it->second.name;
    }
    return nullptr;
}
/*!
    Returns a size of type with \a type ID.
    Returns 0 for unregistered \a type.
*/
int MetaType::size(uint32_t type) {
    PROFILE_FUNCTION();
    auto it = s_Types.find(type);
    if(it != s_Types.end()) {
        return it->second.get_size();
    }
    return 0;
}
/*!
    Constructs a value of the given \a type in the existing memory addressed by \a where, that is a copy of \a copy, and returns where.
    If \a copy is zero, the value is default constructed.
*/
void *MetaType::construct(uint32_t type, void *where, const void *copy) {
    PROFILE_FUNCTION();
    auto it = s_Types.find(type);
    if(it != s_Types.end()) {
        return MetaType(&(it->second)).construct(where, copy);
    }
    return nullptr;
}
/*!
    Returns a copy of \a copy value, with \a type.
    If \a copy is null, creates a default constructed instance.
*/
void *MetaType::create(uint32_t type, const void *copy) {
    PROFILE_FUNCTION();
    auto it = s_Types.find(type);
    if(it != s_Types.end()) {
        return MetaType(&(it->second)).create(copy);
    }
    return nullptr;
}
/*!
    Destroys the value with \a type, located at \a data.
    This function calls delete operator.
*/
void MetaType::destroy(uint32_t type, void *data) {
    PROFILE_FUNCTION();
    auto it = s_Types.find(type);
    if(it != s_Types.end()) {
        MetaType(&(it->second)).destroy(data);
    }
}
/*!
    Destructs the value with \a type, located at \a data.
    Unlike destroy(), this function only invokes the type's destructor, it doesn't invoke the delete operator.
*/
void MetaType::destruct(uint32_t type, void *data) {
    PROFILE_FUNCTION();
    auto it = s_Types.find(type);
    if(it != s_Types.end()) {
        MetaType(&(it->second)).destruct(data);
    }
}
/*!
    Returns true in case of \a left value is equal to \a right value with \a type; otherwise returns false.
*/
bool MetaType::compare(const void *left, const void *right, uint32_t type) {
    PROFILE_FUNCTION();
    auto it = s_Types.find(type);
    if(it != s_Types.end()) {
        return MetaType(&(it->second)).compare(left, right);
    }
    return false;
}
/*!
    Tries to convert value \a from with type \a fromType to type \a toType and place the result to output value \a to.
    Returns true if conversion succeed; otherwise returns false.

    \sa hasConverter()
*/
bool MetaType::convert(const void *from, uint32_t fromType, void *to, uint32_t toType) {
    PROFILE_FUNCTION();
    auto t = s_Converters.find(toType);
    if(t != s_Converters.end()) {
        auto it = t->second.find(fromType);
        if(it != t->second.end()) {
            return (*it->second)(to, from, fromType);
        }
    }
    return false;
}
/*!
    Registers the possibility to convert value type \a from to type \a to with conversion \a function.
    Returns true in case of converter successfully registered; otherwise returns false.

    \sa hasConverter()
*/
bool MetaType::registerConverter(uint32_t from, uint32_t to, converterCallback function) {
    PROFILE_FUNCTION();
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
/*!
    Returns true in case of type \a from can be converted to type \a to; otherwise returns false.
*/
bool MetaType::hasConverter(uint32_t from, uint32_t to) {
    PROFILE_FUNCTION();
    auto t = s_Converters.find(to);
    if(t != s_Converters.end()) {
        auto it = t->second.find(from);
        if(it != t->second.end()) {
            return true;
        }
    }
    return false;
}
/*!
    Returns type information table if type registered; otherwise returns nullptr.
*/
MetaType::Table *MetaType::table(uint32_t type) {
    PROFILE_FUNCTION();
    auto it = s_Types.find(type);
    if(it != s_Types.end()) {
        return &(it->second);
    }
    return nullptr;
}
/*!
    Returns a table of registered types.
*/
MetaType::TypeMap MetaType::types() {
    return s_Types;
}
