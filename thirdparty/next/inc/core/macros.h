/*
    This file is part of Thunder Next.

    Thunder Next is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

    Thunder Next is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Thunder Next.  If not, see <http://www.gnu.org/licenses/>.

    Copyright: 2008-2025 Evgeniy Prikazchikov
*/

#ifndef MACROS_H
#define MACROS_H

#define _SIGNAL(a)  "1"#a
#define _SLOT(a)    "2"#a

#define REGISTER_META_TYPE(Class) \
    REGISTER_META_TYPE_IMPL(Class); \
    REGISTER_META_TYPE_IMPL(Class *);

#define REGISTER_META_TYPE_IMPL(Class) registerMetaType<Class>(#Class)

#define UNREGISTER_META_TYPE(Class) \
    UNREGISTER_META_TYPE_IMPL(Class); \
    UNREGISTER_META_TYPE_IMPL(Class *);

#define UNREGISTER_META_TYPE_IMPL(Class) unregisterMetaType<Class>(#Class)

#define OBJECT_CHECK(Class) \
    static_assert(std::is_base_of<Object, Class>::value, "Class " #Class " should inherit from Object");

#define A_GENERIC(Class) \
private: \
    static void *construct() { return new Class(); } \
public: \
    static const MetaObject *metaClass() { \
        static const MetaObject staticMetaData ( \
            #Class, \
            nullptr, \
            &Class::construct, \
            reinterpret_cast<const MetaMethod::Table *>(expose_method<Class>::exec()), \
            reinterpret_cast<const MetaProperty::Table *>(expose_props_method<Class>::exec()), \
            reinterpret_cast<const MetaEnum::Table *>(expose_enum<Class>::exec()) \
        ); \
        return &staticMetaData; \
    } \
    virtual const MetaObject *metaObject() const { \
        return Class::metaClass(); \
    } \
    static void declareMetaType() { \
        REGISTER_META_TYPE(Class); \
    }

#define A_OBJECT(Class, Super, Group) \
private: \
    static void *construct() { return new Class(); } \
public: \
    static const MetaObject *metaClass() { \
        OBJECT_CHECK(Class) \
        static const MetaObject staticMetaData ( \
            #Class, \
            Super::metaClass(), \
            &Class::construct, \
            reinterpret_cast<const MetaMethod::Table *>(expose_method<Class>::exec()), \
            reinterpret_cast<const MetaProperty::Table *>(expose_props_method<Class>::exec()), \
            reinterpret_cast<const MetaEnum::Table *>(expose_enum<Class>::exec()) \
        ); \
        return &staticMetaData; \
    } \
    const MetaObject *metaObject() const override { \
        return Class::metaClass(); \
    } \
    static void registerClassFactory(ObjectSystem *system) { \
        REGISTER_META_TYPE(Class); \
        system->factoryAdd<Class>(#Group, Class::metaClass()); \
    } \
    static void unregisterClassFactory(ObjectSystem *system) { \
        UNREGISTER_META_TYPE(Class); \
        system->factoryRemove<Class>(#Group); \
    }

#define A_OBJECT_OVERRIDE(Class, Super, Group) \
private: \
    static void *construct() { return new Class(); } \
public: \
    static const MetaObject *metaClass() { \
        OBJECT_CHECK(Class) \
        static const MetaObject staticMetaData ( \
            #Class, \
            Super::metaClass(), \
            &Class::construct, \
            reinterpret_cast<const MetaMethod::Table *>(expose_method<Class>::exec()), \
            reinterpret_cast<const MetaProperty::Table *>(expose_props_method<Class>::exec()), \
            reinterpret_cast<const MetaEnum::Table *>(expose_enum<Class>::exec()) \
        ); \
        return &staticMetaData; \
    } \
    const MetaObject *metaObject() const override { \
        return Super::metaClass(); \
    } \
    static void registerClassFactory(ObjectSystem *system) { \
        system->factoryAdd<Super>(#Group, Class::metaClass()); \
    } \
    static void unregisterClassFactory(ObjectSystem *system) { \
        system->factoryRemove<Super>(#Group); \
        system->factoryAdd<Super>(#Group, Super::metaClass()); \
    } \
    String typeName() const override { \
        return Super::metaClass()->name(); \
    }

// Property declaration
#define A_PROPERTIES(...) \
public: \
    static const MetaProperty::Table *properties() { \
        static const MetaProperty::Table table[] { \
            __VA_ARGS__, \
            {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr} \
        }; \
        return table; \
    }

#define A_NOPROPERTIES() \
public: \
    static const MetaProperty::Table *properties() { \
        static const MetaProperty::Table table[] { \
            {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr} \
        }; \
        return table; \
    }

#define A_PROPERTY(t, p, r, w) { \
    #p, \
    Reader<decltype(&r), &r>::type(#t), \
    nullptr, \
   &Reader<decltype(&r), &r>::read, \
   &Writer<decltype(&w), &w>::write, \
   &Reader<decltype(&r), &r>::address<&r>, \
   &Writer<decltype(&w), &w>::address<&w>, \
    nullptr, \
    nullptr \
}

#define A_PROPERTYEX(t, p, r, w, a) { \
    #p, \
    Reader<decltype(&r), &r>::type(#t), \
    a, \
   &Reader<decltype(&r), &r>::read, \
   &Writer<decltype(&w), &w>::write, \
   &Reader<decltype(&r), &r>::address<&r>, \
   &Writer<decltype(&w), &w>::address<&w>, \
    nullptr, \
    nullptr \
}

// Method declaration
#define A_METHODS(...) \
public: \
    static const MetaMethod::Table *methods() { \
        static const MetaMethod::Table table[] { \
            __VA_ARGS__, \
            {MetaMethod::Method, nullptr, nullptr, nullptr, 0, 0, nullptr} \
        }; \
        return table; \
    }

#define A_NOMETHODS() \
public: \
    static const MetaMethod::Table *methods() { \
        static const MetaMethod::Table table[] { \
            {MetaMethod::Method, nullptr, nullptr, nullptr, 0, 0, nullptr} \
        }; \
        return table; \
    }

#define A_STATIC(r, m) { \
    MetaMethod::Static, \
    #m, \
    (MetaMethod::Table::InvokeMem)&Invoker<decltype(&m)>::invoke<&m>, \
    (MetaMethod::Table::AddressMem)&Invoker<decltype(&m)>::address<&m>, \
    Invoker<decltype(&m)>::signature(#m), \
    Invoker<decltype(&m)>::argCount(), \
    Invoker<decltype(&m)>::types(#r), \
}

#define A_METHOD(r, m) { \
    MetaMethod::Method, \
    #m, \
    (MetaMethod::Table::InvokeMem)&Invoker<decltype(&m)>::invoke<&m>, \
    (MetaMethod::Table::AddressMem)&Invoker<decltype(&m)>::address<&m>, \
    Invoker<decltype(&m)>::signature(#m), \
    Invoker<decltype(&m)>::argCount(), \
    Invoker<decltype(&m)>::types(#r), \
}

#define A_SIGNAL(m) { \
    MetaMethod::Signal, \
    #m, \
    nullptr, \
    nullptr, \
    Invoker<decltype(&m)>::signature(#m), \
    Invoker<decltype(&m)>::argCount(), \
    Invoker<decltype(&m)>::types("void"), \
}

#define A_SLOT(m) { \
    MetaMethod::Slot, \
    #m, \
    (MetaMethod::Table::InvokeMem)&Invoker<decltype(&m)>::invoke<&m>, \
    (MetaMethod::Table::AddressMem)&Invoker<decltype(&m)>::address<&m>, \
    Invoker<decltype(&m)>::signature(#m), \
    Invoker<decltype(&m)>::argCount(), \
    Invoker<decltype(&m)>::types("void"), \
}

#define A_SLOTEX(m, n) { \
    MetaMethod::Slot, \
    n, \
    (MetaMethod::Table::InvokeMem)&Invoker<decltype(&m)>::invoke<&m>, \
    (MetaMethod::Table::AddressMem)&Invoker<decltype(&m)>::address<&m>, \
    0, \
    Invoker<decltype(&m)>::argCount(), \
    Invoker<decltype(&m)>::types("void"), \
}

// Enumerator declaration
#define A_ENUMS(...) \
public: \
    static const MetaEnum::Table *enums() { \
        static const MetaEnum::Table table[] { \
            __VA_ARGS__, \
            {nullptr, nullptr} \
        }; \
        return table; \
    }

#define A_NOENUMS() \
public: \
    static const MetaEnum::Table *enums() { \
        static const MetaEnum::Table table[] { \
            {nullptr, nullptr} \
        }; \
        return table; \
    }

#define A_ENUM(Name, ...) { \
    #Name, \
    [](){ \
        static const MetaEnum::EnumTable table[] { \
            __VA_ARGS__, \
            {nullptr, 0} \
        }; \
        return table; \
    }() \
}

#define A_VALUE(Name) { #Name, Name }

#endif
