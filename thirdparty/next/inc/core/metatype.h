#ifndef METATYPE_H
#define METATYPE_H

#include <map>
#include <unordered_map>
#include <typeinfo>
#include <typeindex>
#include <stdint.h>

#include "global.h"
#include "macros.h"

using namespace std;

class Object;

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
        RAY,

        OBJECT                  = 30,

        USERTYPE                = 40
    };

    /*! \enum Flags */
    enum Flags {
        POINTER     = (1<<0),
        BASE_OBJECT = (1<<1)
    };

    struct Table {
        const void         *properties;
        const void         *methods;
        const void         *enums;
        int                 (*get_size)                 ();
        void               *(*static_new)               ();
        void                (*construct)                (void *);
        void                (*static_delete)            (void **);
        void                (*destruct)                 (void *);
        void                (*clone)                    (const void **, void **);
        bool                (*compare)                  (const void **, const void **);
        type_index const    (*index)                    ();
        const char         *name;
        int                 flags;
    };

    typedef bool            (*converterCallback)        (void *to, const void *from, const uint32_t fromType);

    typedef unordered_map<uint32_t, Table>              TypeMap;

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
    int                     flags                       () const;

    static uint32_t         registerType                (Table &table);
    static void             unregisterType              (Table &table);

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

    static Table           *table                       (uint32_t type);

    static TypeMap          types                       ();

private:
    const Table            *m_pTable;

    static uint32_t         s_NextId;
};

template<typename T>
struct TypeFuncs {
    static int size() {
        return sizeof(T);
    }
    static void *static_new() {
        return new T();
    }
    static void static_delete(void **x) {
        delete (*reinterpret_cast<T **>(x));
    }
    static void construct(void *dest) {
        new (dest) T();
    }
    static void destruct(void *x) {
        (reinterpret_cast<T *>(x))->~T();
    }
    static void clone(const void **src, void **dest) {
        *dest = new T(**reinterpret_cast<const T **>(src));
    }
    static bool compare(const void **left, const void **right) {
        return (**reinterpret_cast<const T **>(left) == **reinterpret_cast<const T **>(right));
    }
    static type_index const index() {
        return type_index(typeid(T));
    }
};

template<typename T>
struct expose_method {
private:
    typedef std::true_type yes;
    typedef std::false_type no;
    template<typename U>
    static auto test(int) -> decltype(std::declval<U>().methods(), yes()) {
        return yes();
    }
    template<typename>
    static no test(...) {
        return no();
    }
    static const void *exec_impl(std::true_type) {
        return T::methods();
    }
    static const void *exec_impl(...) {
        return nullptr;
    }
public:
    static const void *exec() {
        return exec_impl(test<T>(0));
    }
    enum { exists = std::is_same<decltype(test<T>(0)), yes>::value };
};

template<typename T>
struct expose_enum {
private:
    typedef std::true_type yes;
    typedef std::false_type no;
    template<typename U>
    static auto test(int) -> decltype(std::declval<U>().enums(), yes()) {
        return yes();
    }
    template<typename>
    static no test(...) {
        return no();
    }
    static const void *exec_impl(std::true_type) {
        return T::enums();
    }
    static const void *exec_impl(...) {
        return nullptr;
    }
public:
    static const void *exec() {
        return exec_impl(test<T>(0));
    }
    enum { exists = std::is_same<decltype(test<T>(0)), yes>::value };
};

template<typename T>
struct expose_props_method {
private:
    typedef std::true_type yes;
    typedef std::false_type no;
    template<typename U>
    static auto test(int) -> decltype(std::declval<U>().properties(), yes()) {
        return yes();
    }
    template<typename>
    static no test(...) {
        return no();
    }
    static const void *exec_impl(std::true_type) {
        return T::properties();
    }
    static const void *exec_impl(...) {
        return nullptr;
    }
public:
    static const void *exec() {
        return exec_impl(test<T>(0));
    }
    enum { exists = std::is_same<decltype(test<T>(0)), yes>::value };
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
    typedef typename std::remove_cv<
            typename std::remove_reference<T>
            ::type>::type type;
};

template<typename T>
struct Table {
    typedef Bool<std::is_pointer<T>::value>         is_pointer;
    typedef typename CheckType<T, is_pointer>::type T_no_cv;

    static MetaType::Table *get(const char *typeName, int flags = 0) {
        static MetaType::Table staticTable = {
            expose_props_method<T_no_cv>::exec(),
            expose_method<T_no_cv>::exec(),
            expose_enum<T_no_cv>::exec(),
            TypeFuncs<T_no_cv>::size,
            TypeFuncs<T_no_cv>::static_new,
            TypeFuncs<T_no_cv>::construct,
            TypeFuncs<T_no_cv>::static_delete,
            TypeFuncs<T_no_cv>::destruct,
            TypeFuncs<T_no_cv>::clone,
            TypeFuncs<T_no_cv>::compare,
            TypeFuncs<T_no_cv>::index,
            typeName,
            flags
        };
        return &staticTable;
    };
};

//Function to unpack args properly
template<typename T>
inline static MetaType::Table *getTable(const char *typeName = "") {
    uint32_t type = MetaType::type<T>();
    MetaType::Table *result = MetaType::table(type);
    if(result) {
        return result;
    }

    int flags = 0;
    if(std::is_pointer<T>::value) {
        flags |= MetaType::POINTER;

        if(std::is_base_of<Object, typename std::remove_pointer<T>::type>::value) {
            flags |= MetaType::BASE_OBJECT;
        }
    }

    return Table<T>::get(typeName, flags);
}

template<typename T>
static uint32_t registerMetaType(const char *typeName) {
    return MetaType::registerType(*getTable<T>(typeName));
}

template<typename T>
static void unregisterMetaType(const char *typeName) {
    MetaType::unregisterType(*getTable<T>(typeName));
}

#endif // METATYPE_H
