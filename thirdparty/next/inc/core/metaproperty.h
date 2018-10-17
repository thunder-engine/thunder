#ifndef METAPROPERTY_H
#define METAPROPERTY_H

#include "variant.h"
#include "metatype.h"

class Object;

class NEXT_LIBRARY_EXPORT MetaProperty {
public:
    typedef Variant             (*ReadMem)              (const Object *);
    typedef void                (*WriteMem)             (Object *, const Variant&);
    typedef void                (*AddressMem)           (char *, size_t);

    struct Table {
        const char             *name;
        const MetaType::Table  *type;
        ReadMem                 reader;
        WriteMem                writer;
        AddressMem              readmem;
        AddressMem              writemem;
    };

public:
    MetaProperty            (const Table *table);

    const char             *name                        () const;
    bool                    isValid                     () const;
    const MetaType          type                        () const;

    Variant                 read                        (const Object *object) const;
    void                    write                       (Object *object, const Variant &value) const;

    template<typename T>
    void                    write                       (Object *object, const T &value) const {
        uint32_t type   = MetaType::type<T>();
        Variant arg;
        if(type < MetaType::VARIANTMAP && type >= MetaType::USERTYPE) {
            arg = Variant::fromValue<T>(value);
        } else {
            arg = Variant(value);
        }
        write(object, arg);
    }

    const Table            *table                       () const;

private:
    const Table            *m_pTable;

};

//Property read
template<typename Signature, Signature S>
struct Reader;

template<typename T, typename Class, T(Class::*ReadFunc)()>
struct Reader<T(Class::*)(), ReadFunc> {
    typedef T(Class::*Fun)();

    typedef Bool<std::is_pointer<T>::value>         is_pointer;
    typedef typename CheckType<T, is_pointer>::type T_no_cv;

    inline static const MetaType::Table *type(const char *typeName) {
        return Table<T>::get(typeName);
    }

    inline static Variant read(const Object *obj) {
        return Variant::fromValue<T_no_cv>((static_cast<Class *>(obj)->*ReadFunc)());
    }

    template<Fun fun>
    static void address(char *ptr, size_t size) {
        Fun f   = fun;
        for(size_t n = 0; n < size; n++) {
            ptr[n]  = reinterpret_cast<const char *>(&f)[n];
        }
    }
};
template<typename T, typename Class, T(Class::*ReadFunc)()const>
struct Reader<T(Class::*)()const, ReadFunc> {
    typedef T(Class::*Fun)()const;

    typedef Bool<std::is_pointer<T>::value>         is_pointer;
    typedef typename CheckType<T, is_pointer>::type T_no_cv;

    inline static const MetaType::Table *type(const char *typeName) {
        return Table<T>::get(typeName);
    }

    inline static Variant read(const Object *obj) {
        return Variant::fromValue<T_no_cv>((static_cast<const Class *>(obj)->*ReadFunc)());
    }

    template<Fun fun>
    static void address(char *ptr, size_t size) {
        Fun f   = fun;
        for(size_t n = 0; n < size; n++) {
            ptr[n]  = reinterpret_cast<const char *>(&f)[n];
        }
    }
};

//Property write
template<typename Signature, Signature S>
struct Writer;

template<typename T, typename Class, void(Class::*WriteFunc)(T)>
struct Writer<void(Class::*)(T), WriteFunc> {
    typedef void(Class::*Fun)(T);

    typedef Bool<std::is_pointer<T>::value>         is_pointer;
    typedef typename CheckType<T, is_pointer>::type T_no_cv;

    inline static void write(Object *obj, const Variant &value) {
        return (static_cast<Class *>(obj)->*WriteFunc)(value.value<T_no_cv>());
    }

    template<Fun fun>
    static void address(char *ptr, size_t size) {
        Fun f   = fun;
        for(size_t n = 0; n < size; n++) {
            ptr[n]  = reinterpret_cast<const char *>(&f)[n];
        }
    }
};
template<typename T, typename Class, void(Class::*WriteFunc)(const T&)>
struct Writer<void(Class::*)(const T&), WriteFunc> {
    typedef void(Class::*Fun)(const T&);

    typedef Bool<std::is_pointer<T>::value>         is_pointer;
    typedef typename CheckType<T, is_pointer>::type T_no_cv;

    inline static void write(Object *obj, const Variant &value) {
        return (static_cast<Class *>(obj)->*WriteFunc)(value.value<T_no_cv>());
    }

    template<Fun fun>
    inline static void address(char *ptr, size_t size) {
        Fun f   = fun;
        for(size_t n = 0; n < size; n++) {
            ptr[n]  = reinterpret_cast<const char *>(&f)[n];
        }
    }
};

#endif // AMETAPROPERTY_H
