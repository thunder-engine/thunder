#ifndef METAOBJECT_H
#define METAOBJECT_H

#include <string>

#include "metatype.h"
#include "metaproperty.h"
#include "metamethod.h"

#define OBJECT_CHECK(Class) \
    static_assert(std::is_base_of<Object, Class>::value, "Class " #Class " should inherit from Object");

#define A_OBJECT(Class, Super) \
private: \
    static Object *construct() { return new Class(); } \
public: \
    static const MetaObject *metaClass() { \
        OBJECT_CHECK(Class) \
        static const MetaObject staticMetaData ( \
            #Class, \
            Super::metaClass(), \
            &Class::construct, \
            expose_method<Class>::exec(), \
            expose_props_method<Class>::exec() \
        ); \
        return &staticMetaData; \
    } \
    virtual const MetaObject *metaObject() const { \
        return Class::metaClass(); \
    }

#define A_PROPERTIES(...) \
public: \
    static const MetaProperty::Table *properties() { \
        static const MetaProperty::Table table[] { \
            __VA_ARGS__, \
            {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr} \
        }; \
        return table; \
    }

#define A_NOPROPERTIES() \
public: \
    static const MetaProperty::Table *properties() { \
        static const MetaProperty::Table table[] { \
            {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr} \
        }; \
        return table; \
    }

#define A_METHODS(...) \
public: \
    static const MetaMethod::Table *methods() { \
        static const MetaMethod::Table table[] { \
            __VA_ARGS__, \
            {MetaMethod::Method, nullptr, nullptr, nullptr, 0, nullptr} \
        }; \
        return table; \
    }

#define A_NOMETHODS() \
public: \
    static const MetaMethod::Table *methods() { \
        static const MetaMethod::Table table[] { \
            {MetaMethod::Method, nullptr, nullptr, nullptr, 0, nullptr} \
        }; \
        return table; \
    }

#define A_PROPERTY(t, p, r, w) \
{ \
    #p, \
    Reader<decltype(&r), &r>::type(#t), \
   &Reader<decltype(&r), &r>::read, \
   &Writer<decltype(&w), &w>::write, \
   &Reader<decltype(&r), &r>::address<&r>, \
   &Writer<decltype(&w), &w>::address<&w>, \
    nullptr \
}

#define A_METHOD(r, m) { \
    MetaMethod::Method, \
    #m, \
    (MetaMethod::Table::InvokeMem)&Invoker<decltype(&m)>::invoke<&m>, \
    (MetaMethod::Table::AddressMem)&Invoker<decltype(&m)>::address<&m>, \
    Invoker<decltype(&m)>::argCount(), \
    Invoker<decltype(&m)>::types(#r), \
}

#define A_SIGNAL(m) { \
    MetaMethod::Signal, \
    #m, \
    nullptr, \
    nullptr, \
    Invoker<decltype(&m)>::argCount(), \
    Invoker<decltype(&m)>::types("void"), \
}

#define A_SLOT(m) { \
    MetaMethod::Slot, \
    #m, \
    (MetaMethod::Table::InvokeMem)&Invoker<decltype(&m)>::invoke<&m>, \
    (MetaMethod::Table::AddressMem)&Invoker<decltype(&m)>::address<&m>, \
    Invoker<decltype(&m)>::argCount(), \
    Invoker<decltype(&m)>::types("void"), \
}

#define _SIGNAL(a)  "1"#a
#define _SLOT(a)    "2"#a

class Object;

class NEXT_LIBRARY_EXPORT MetaObject {
public:
    typedef Object             *(*Constructor)              ();

public:
    explicit MetaObject         (const char *, const MetaObject *, const Constructor, const MetaMethod::Table *, const MetaProperty::Table *);

    const char                 *name                        () const;
    const MetaObject           *super                       () const;

    Object                     *createInstance              () const;

    int                         indexOfMethod               (const char *) const;
    int                         indexOfSignal               (const char *) const;
    int                         indexOfSlot                 (const char *) const;

    MetaMethod                  method                      (int) const;
    int                         methodCount                 () const;
    int                         methodOffset                () const;

    int                         indexOfProperty             (const char *) const;
    MetaProperty                property                    (int) const;
    int                         propertyCount               () const;
    int                         propertyOffset              () const;
    bool                        canCastTo                   (const char *) const;

private:
    Constructor                 m_Constructor;
    const char                 *m_pName;
    const MetaObject           *m_pSuper;
    const MetaMethod::Table    *m_pMethods;
    const MetaProperty::Table  *m_pProperties;
    int                         m_MethodCount;
    int                         m_PropCount;

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
    static const MetaMethod::Table *exec_impl(std::true_type) {
        return T::methods();
    }
    static const MetaMethod::Table *exec_impl(...) {
        return nullptr;
    }
public:
    static const MetaMethod::Table *exec() {
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
    static const MetaProperty::Table *exec_impl(std::true_type) {
        return T::properties();
    }
    static const MetaProperty::Table *exec_impl(...) {
        return nullptr;
    }
public:
    static const MetaProperty::Table *exec() {
        return exec_impl(test<T>(0));
    }
    enum { exists = std::is_same<decltype(test<T>(0)), yes>::value };
};

#endif // METAOBJECT_H
