#ifndef AMETAPROPERTY_H
#define AMETAPROPERTY_H

#include "avariant.h"
#include "ametatype.h"

class AObject;

class NEXT_LIBRARY_EXPORT AMetaProperty {
public:
    typedef AVariant            (*ReadMem)              (const AObject *);
    typedef void                (*WriteMem)             (AObject *, const AVariant&);

    struct Table {
        const char             *name;
        const AMetaType::Table *type;
        ReadMem                 reader;
        WriteMem                writer;
    };

public:
    AMetaProperty           (const Table *table);

    const char             *name                        () const;
    bool                    isValid                     () const;
    const AMetaType         type                        () const;

    AVariant                read                        (const AObject *obj) const;
    void                    write                       (AObject *obj, const AVariant &value) const;

    template<typename T>
    void                    write                       (AObject *obj, const T &value) const {
        uint32_t type   = AMetaType::type<T>();
        AVariant arg;
        if(type < AMetaType::VARIANTMAP && type >= AMetaType::USERTYPE) {
            arg = AVariant::fromValue<T>(value);
        } else {
            arg = AVariant(value);
        }
        write(obj, arg);
    }

private:
    const Table            *m_pTable;

};

//Property read
template<typename Signature, Signature S>
struct Reader;

template<typename T, typename Class, T(Class::*ReadFunc)()>
struct Reader<T(Class::*)(), ReadFunc> {
    inline static const AMetaType::Table *type(const char *typeName) {
        return Table<T>::get(typeName);
    }

    inline static AVariant read(const AObject *obj) {
        return AVariant::fromValue<T>((static_cast<Class *>(obj)->*ReadFunc)());
    }
};
template<typename T, typename Class, T(Class::*ReadFunc)()const>
struct Reader<T(Class::*)()const, ReadFunc> {
    inline static const AMetaType::Table *type(const char *typeName) {
        return Table<T>::get(typeName);
    }

    inline static AVariant read(const AObject *obj) {
        return AVariant::fromValue<T>((static_cast<const Class *>(obj)->*ReadFunc)());
    }
};

//Property write
template<typename Signature, Signature S>
struct Writer;

template<typename T, typename Class, void(Class::*WriteFunc)(T)>
struct Writer<void(Class::*)(T), WriteFunc> {
    inline static void write(AObject *obj, const AVariant &value) {
        return (static_cast<Class *>(obj)->*WriteFunc)(value.value<T>());
    }
};
template<typename T, typename Class, void(Class::*WriteFunc)(const T&)>
struct Writer<void(Class::*)(const T&), WriteFunc> {
    inline static void write(AObject *obj, const AVariant &value) {
        return (static_cast<Class *>(obj)->*WriteFunc)(value.value<T>());
    }
};

#endif // AMETAPROPERTY_H
