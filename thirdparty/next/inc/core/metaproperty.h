#ifndef METAPROPERTY_H
#define METAPROPERTY_H

#include "variant.h"
#include "metatype.h"

class Object;

class NEXT_LIBRARY_EXPORT MetaProperty {
public:
    typedef Variant(*ReadMem)(const void *);
    typedef void(*WriteMem)(void *, const Variant&);
    typedef void(*AddressMem)(char *, size_t);
    typedef Variant(*ReadProperty)(const void *, const MetaProperty&);
    typedef void(*WriteProperty)(void *, const MetaProperty&, const Variant&);

    struct Table {
        const char *name;
        const MetaType::Table *type;
        const char *annotation;
        ReadMem reader;
        WriteMem writer;
        AddressMem readmem;
        AddressMem writemem;
        ReadProperty readproperty;
        WriteProperty writeproperty;
    };

public:
    explicit MetaProperty(const Table *table);

    const char *name() const;
    bool isValid() const;
    const MetaType type() const;

    Variant read(const void *object) const;
    void write(void *object, const Variant &value) const;

    template<typename T>
    void write(void *object, const T &value) const {
        uint32_t type = MetaType::type<T>();
        Variant arg;
        if(type < MetaType::VARIANTMAP && type >= MetaType::USERTYPE) {
            arg = Variant::fromValue<T>(value);
        } else {
            arg = Variant(value);
        }
        write(object, arg);
    }

    const Table *table() const;

private:
    const Table *m_table;

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

    inline static Variant read(const void *obj) {
        return Variant::fromValue<T_no_cv>((reinterpret_cast<Class *>(obj)->*ReadFunc)());
    }

    template<Fun fun>
    static void address(char *ptr, size_t size) {
        Fun f = fun;
        for(size_t n = 0; n < size; n++) {
            ptr[n] = reinterpret_cast<const char *>(&f)[n];
        }
    };
};
template<typename T, typename Class, T(Class::*ReadFunc)()const>
struct Reader<T(Class::*)()const, ReadFunc> {
    typedef T(Class::*Fun)()const;

    typedef Bool<std::is_pointer<T>::value>         is_pointer;
    typedef typename CheckType<T, is_pointer>::type T_no_cv;

    inline static const MetaType::Table *type(const char *typeName) {
        return Table<T>::get(typeName);
    }

    inline static Variant read(const void *obj) {
        auto value =(reinterpret_cast<const Class *>(obj)->*ReadFunc)();
        return Variant(MetaType::type<decltype(value)>(), &value);
    }

    template<Fun fun>
    static void address(char *ptr, size_t size) {
        Fun f = fun;
        for(size_t n = 0; n < size; n++) {
            ptr[n] = reinterpret_cast<const char *>(&f)[n];
        }
    };
};

template<typename T, typename Class, T(Class::*ReadFunc)(const MetaProperty &)>
struct Reader<T(Class::*)(const MetaProperty &), ReadFunc> {
    typedef Bool<std::is_pointer<T>::value>         is_pointer;
    typedef typename CheckType<T, is_pointer>::type T_no_cv;

    inline static const MetaType::Table *type(const char *typeName) {
        return Table<T>::get(typeName);
    }

    inline static Variant read(const void *obj, const MetaProperty &property) {
        return(reinterpret_cast<Class *>(obj)->*ReadFunc)(property);
    }
};
template<typename T, typename Class, T(Class::*ReadFunc)(const MetaProperty &)const>
struct Reader<T(Class::*)(const MetaProperty &)const, ReadFunc> {
    typedef Bool<std::is_pointer<T>::value>         is_pointer;
    typedef typename CheckType<T, is_pointer>::type T_no_cv;

    inline static const MetaType::Table *type(const char *typeName) {
        return Table<T>::get(typeName);
    }

    inline static Variant read(const void *obj, const MetaProperty &property) {
        return(reinterpret_cast<const Class *>(obj)->*ReadFunc)(property);
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

    inline static void write(void *obj, const Variant &value) {
        return(reinterpret_cast<Class *>(obj)->*WriteFunc)(value.value<T_no_cv>());
    }

    template<Fun fun>
    static void address(char *ptr, size_t size) {
        Fun f = fun;
        for(size_t n = 0; n < size; n++) {
            ptr[n] = reinterpret_cast<const char *>(&f)[n];
        }
    }
};
template<typename T, typename Class, void(Class::*WriteFunc)(const T&)>
struct Writer<void(Class::*)(const T&), WriteFunc> {
    typedef void(Class::*Fun)(const T&);

    typedef Bool<std::is_pointer<T>::value>         is_pointer;
    typedef typename CheckType<T, is_pointer>::type T_no_cv;

    inline static void write(void *obj, const Variant &value) {
        return(reinterpret_cast<Class *>(obj)->*WriteFunc)(value.value<T_no_cv>());
    }

    template<Fun fun>
    inline static void address(char *ptr, size_t size) {
        Fun f = fun;
        for(size_t n = 0; n < size; n++) {
            ptr[n] = reinterpret_cast<const char *>(&f)[n];
        }
    }
};
template<typename T, typename Class, void(Class::*WriteFunc)(const MetaProperty &, T)>
struct Writer<void(Class::*)(const MetaProperty &, T), WriteFunc> {
    typedef Bool<std::is_pointer<T>::value>         is_pointer;
    typedef typename CheckType<T, is_pointer>::type T_no_cv;

    inline static void write(void *obj, const MetaProperty &property, const Variant &value) {
        return(reinterpret_cast<Class *>(obj)->*WriteFunc)(property, value);
    }
};
template<typename T, typename Class, void(Class::*WriteFunc)(const MetaProperty &, const T&)>
struct Writer<void(Class::*)(const MetaProperty &, const T&), WriteFunc> {
    typedef Bool<std::is_pointer<T>::value>         is_pointer;
    typedef typename CheckType<T, is_pointer>::type T_no_cv;

    inline static void write(void *obj, const MetaProperty &property, const Variant &value) {
(reinterpret_cast<Class *>(obj)->*WriteFunc)(property, value);
    }
};

#endif // AMETAPROPERTY_H
