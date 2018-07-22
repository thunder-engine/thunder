#ifndef METAPROPERTY_H
#define METAPROPERTY_H

#include "variant.h"
#include "metatype.h"

class Object;

class NEXT_LIBRARY_EXPORT MetaProperty {
public:
    typedef Variant             (*ReadMem)              (const Object *);
    typedef void                (*WriteMem)             (Object *, const Variant&);

    struct Table {
        const char             *name;
        const MetaType::Table *type;
        ReadMem                 reader;
        WriteMem                writer;
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

private:
    const Table            *m_pTable;

};

//Property read
template<typename Signature, Signature S>
struct Reader;

template<typename T, typename Class, T(Class::*ReadFunc)()>
struct Reader<T(Class::*)(), ReadFunc> {
    inline static const MetaType::Table *type(const char *typeName) {
        return Table<T>::get(typeName);
    }

    inline static Variant read(const Object *obj) {
        return Variant::fromValue<T>((static_cast<Class *>(obj)->*ReadFunc)());
    }
};
template<typename T, typename Class, T(Class::*ReadFunc)()const>
struct Reader<T(Class::*)()const, ReadFunc> {
    inline static const MetaType::Table *type(const char *typeName) {
        return Table<T>::get(typeName);
    }

    inline static Variant read(const Object *obj) {
        return Variant::fromValue<T>((static_cast<const Class *>(obj)->*ReadFunc)());
    }
};

//Property write
template<typename Signature, Signature S>
struct Writer;

template<typename T, typename Class, void(Class::*WriteFunc)(T)>
struct Writer<void(Class::*)(T), WriteFunc> {
    inline static void write(Object *obj, const Variant &value) {
        return (static_cast<Class *>(obj)->*WriteFunc)(value.value<T>());
    }
};
template<typename T, typename Class, void(Class::*WriteFunc)(const T&)>
struct Writer<void(Class::*)(const T&), WriteFunc> {
    inline static void write(Object *obj, const Variant &value) {
        return (static_cast<Class *>(obj)->*WriteFunc)(value.value<T>());
    }
};

#endif // AMETAPROPERTY_H
