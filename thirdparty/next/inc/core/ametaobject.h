#ifndef AMETAOBJECT_H
#define AMETAOBJECT_H

#include <string>

#include "ametatype.h"
#include "ametaproperty.h"
#include "ametamethod.h"

#define OBJECT_CHECK(Class) \
    static_assert(std::is_base_of<AObject, Class>::value, "Class " #Class " should inherit from AObject");

#define A_OBJECT(Class, Super) \
private: \
    static AObject *construct() { return new Class(); } \
public: \
    static const AMetaObject *metaClass() { \
        OBJECT_CHECK(Class) \
        static const AMetaObject staticMetaData ( \
            #Class, \
            Super::metaClass(), \
            &Class::construct, \
            expose_method<Class>::exec(), \
            expose_props_method<Class>::exec() \
        ); \
        return &staticMetaData; \
    } \
    virtual const AMetaObject *metaObject() const { \
        return Class::metaClass(); \
    }

#define A_PROPERTIES(...) \
public: \
    static const AMetaProperty::Table *properties() { \
        static const AMetaProperty::Table table[] { \
            __VA_ARGS__, \
            {nullptr, nullptr, nullptr, nullptr} \
        }; \
        return table; \
    }

#define A_NOPROPERTIES() \
public: \
    static const AMetaProperty::Table *properties() { \
        static const AMetaProperty::Table table[] { \
            {nullptr, nullptr, nullptr, nullptr} \
        }; \
        return table; \
    }

#define A_METHODS(...) \
public: \
    static const AMetaMethod::Table *methods() { \
        static const AMetaMethod::Table table[] { \
            __VA_ARGS__, \
            {AMetaMethod::Method, nullptr, nullptr, 0, nullptr} \
        }; \
        return table; \
    }

#define A_NOMETHODS() \
public: \
    static const AMetaMethod::Table *methods() { \
        static const AMetaMethod::Table table[] { \
            {AMetaMethod::Method, nullptr, nullptr, 0, nullptr} \
        }; \
        return table; \
    }

#define A_PROPERTY(t, p, r, w) \
{ \
    #p, \
    Reader<decltype(&r), &r>::type(#t), \
   &Reader<decltype(&r), &r>::read, \
   &Writer<decltype(&w), &w>::write \
}

#define A_METHOD(m) { \
    AMetaMethod::Method, \
    #m, \
    (AMetaMethod::InvokeMem)&Invoker<decltype(&m)>::invoke<&m>, \
    Invoker<decltype(&m)>::argCount(), \
    Invoker<decltype(&m)>::types() \
}

#define A_SIGNAL(m) { \
    AMetaMethod::Signal, \
    #m, \
    nullptr, \
    Invoker<decltype(&m)>::argCount(), \
    Invoker<decltype(&m)>::types() \
}

#define A_SLOT(m) { \
    AMetaMethod::Slot, \
    #m, \
    (AMetaMethod::InvokeMem)&Invoker<decltype(&m)>::invoke<&m>, \
    Invoker<decltype(&m)>::argCount(), \
    Invoker<decltype(&m)>::types() \
}

#define _SIGNAL(a)  "1"#a
#define _SLOT(a)    "2"#a

class AObject;

class NEXT_LIBRARY_EXPORT AMetaObject {
public:
    typedef AObject            *(*Constructor)              ();

public:
    AMetaObject                 (const char *, const AMetaObject *, const Constructor, const AMetaMethod::Table *, const AMetaProperty::Table *);

    const char                 *name                        () const;
    const AMetaObject          *super                       () const;

    AObject                    *createInstance              () const;

    int                         indexOfMethod               (const char *) const;
    int                         indexOfSignal               (const char *) const;
    int                         indexOfSlot                 (const char *) const;

    AMetaMethod                 method                      (int) const;
    int                         methodCount                 () const;
    int                         methodOffset                () const;

    int                         indexOfProperty             (const char *) const;
    AMetaProperty               property                    (int) const;
    int                         propertyCount               () const;
    int                         propertyOffset              () const;
    bool                        canCastTo                   (const char *) const;

private:
    Constructor                 m_Constructor;
    const char                 *m_pName;
    const AMetaObject          *m_pSuper;
    const AMetaMethod::Table   *m_pMethods;
    const AMetaProperty::Table *m_pProperties;
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
    static const AMetaMethod::Table *exec_impl(std::true_type) {
        return T::methods();
    }
    static const AMetaMethod::Table *exec_impl(...) {
        return nullptr;
    }
public:
    static const AMetaMethod::Table *exec() {
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
    static const AMetaProperty::Table *exec_impl(std::true_type) {
        return T::properties();
    }
    static const AMetaProperty::Table *exec_impl(...) {
        return nullptr;
    }
public:
    static const AMetaProperty::Table *exec() {
        return exec_impl(test<T>(0));
    }
    enum { exists = std::is_same<decltype(test<T>(0)), yes>::value };
};

#endif // AMETAOBJECT_H
