#ifndef MetaType_H
#define MetaType_H

#include <map>
#include <typeinfo>
#include <typeindex>
#include <stdint.h>

#include <common.h>

using namespace std;

class NEXT_LIBRARY_EXPORT MetaType {
public:
    /*! \enum Type */
    enum Type {
        INVALID                 = 0,
        BOOLEAN,
        INTEGER,
        FLOAT,
        STRING,
        VARIANTMAP,
        VARIANTLIST,
        BYTEARRAY,

        VECTOR2                 = 10,
        VECTOR3,
        VECTOR4,
        QUATERNION,
        MATRIX3,
        MATRIX4,

        USERTYPE                = 20
    };

    struct Table {
        int                 (*get_size)                 ();
        void                (*static_new)               (void**);
        void                (*construct)                (void**);
        void                (*static_delete)            (void**);
        void                (*destruct)                 (void**);
        void                (*clone)                    (const void**, void**);
        void                (*move)                     (const void**, void**);
        bool                (*compare)                  (const void**, const void**);
        type_index const    (*index)                    ();
        const char         *name;
    };

    typedef bool            (*converterCallback)        (void *to, const void *from, const uint32_t fromType);

public:
    MetaType               (const Table *table);

    const char             *name                        () const;
    int                     size                        () const;
    void                   *construct                   (void *where, const void *copy = nullptr) const;
    void                   *create                      (const void *copy = nullptr) const;
    void                    destroy                     (void *data) const;
    void                    destruct                    (void *data) const;
    bool                    compare                     (const void *left, const void *right) const;
    bool                    isValid                     () const;

    static uint32_t         registerType                (Table &table);

    static uint32_t         type                        (const char *name);

    static uint32_t         type                        (const type_info &type);

    template<typename T>
    static uint32_t         type                        () {
        return type(typeid(T));
    }

    static const char      *name                        (uint32_t type);
    static int              size                        (uint32_t type);
    static void            *construct                   (uint32_t type, void *where, const void *copy = nullptr);
    static void            *create                      (uint32_t type, const void *copy = nullptr);
    static void             destroy                     (uint32_t type, void *data);
    static void             destruct                    (uint32_t type, void *data);

    static bool             compare                     (const void *left, const void *right, uint32_t type);
    static bool             convert                     (const void *from, uint32_t fromType, void *to, uint32_t toType);
    static bool             registerConverter           (uint32_t from, uint32_t to, converterCallback function);
    static bool             hasConverter                (uint32_t from, uint32_t to);

    static bool             toBoolean                   (void *to, const void *from, const uint32_t fromType);
    static bool             toInteger                   (void *to, const void *from, const uint32_t fromType);
    static bool             toFloat                     (void *to, const void *from, const uint32_t fromType);
    static bool             toString                    (void *to, const void *from, const uint32_t fromType);
    static bool             toList                      (void *to, const void *from, const uint32_t fromType);
    static bool             toVector2                   (void *to, const void *from, const uint32_t fromType);
    static bool             toVector3                   (void *to, const void *from, const uint32_t fromType);
    static bool             toVector4                   (void *to, const void *from, const uint32_t fromType);
    static bool             toMatrix3                   (void *to, const void *from, const uint32_t fromType);
    static bool             toMatrix4                   (void *to, const void *from, const uint32_t fromType);
    static bool             toQuaternion                (void *to, const void *from, const uint32_t fromType);

private:
    const Table            *m_pTable;

    static uint32_t         s_NextId;
};

template<typename T>
struct TypeFuncs {
    static int size() {
        return sizeof(T);
    }
    static void static_new(void** dest) {
        *dest = new T();
    }
    static void static_delete(void** x) {
        delete (*reinterpret_cast<T**>(x));
    }
    static void construct(void** dest) {
        new (*dest) T();
    }
    static void destruct(void** x) {
        (*reinterpret_cast<T**>(x))->~T();
    }
    static void clone(const void** src, void** dest) {
        *dest = new T(**reinterpret_cast<const T**>(src));
    }
    static void move(const void** src, void** dest) {
        **reinterpret_cast<T**>(dest) =
            **reinterpret_cast<const T**>(src);
    }
    static bool compare(const void** left, const void** right) {
        return (**reinterpret_cast<const T**>(left) == **reinterpret_cast<const T**>(right));
    }
    static type_index const index() {
        return type_index(typeid(T));
    }
};


//Bool template type
template<bool B> struct Bool;
typedef Bool<true> True;
typedef Bool<false> False;

//Bool implementation
template<bool B>
struct Bool {
    static const bool value = B;
    typedef Bool type;
};

template<typename T, typename Ptr>
struct CheckType;

template<typename T>
struct CheckType<T, True> {
    typedef typename std::add_pointer<
            typename std::remove_cv<
            typename std::remove_pointer<T>
            ::type>::type>::type type;
};

template<typename T>
struct CheckType<T, False> {
    typedef typename std::remove_cv<T>::type type;
};

template<typename T>
struct Table {
    typedef Bool<std::is_pointer<T>::value>         is_pointer;
    typedef typename CheckType<T, is_pointer>::type T_no_cv;

    static MetaType::Table *get(const char *typeName) {
        static MetaType::Table staticTable = {
            TypeFuncs<T_no_cv>::size,
            TypeFuncs<T_no_cv>::static_new,
            TypeFuncs<T_no_cv>::construct,
            TypeFuncs<T_no_cv>::static_delete,
            TypeFuncs<T_no_cv>::destruct,
            TypeFuncs<T_no_cv>::clone,
            TypeFuncs<T_no_cv>::move,
            TypeFuncs<T_no_cv>::compare,
            TypeFuncs<T_no_cv>::index,
            typeName
        };
        return &staticTable;
    }
};

//Function to unpack args properly
template<typename T>
inline static MetaType::Table *getTable(const char *typeName) {
    return Table<T>::get(typeName);
}

template<typename T>
static uint32_t registerMetaType(const char *typeName) {
    return MetaType::registerType(*getTable<T>(typeName));
}

#endif // MetaType_H
