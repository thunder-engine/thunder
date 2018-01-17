#ifndef AMETAMETHOD_H
#define AMETAMETHOD_H

#include <string>

#include "avariant.h"
#include "ametatype.h"
#include "aevent.h"

using namespace std;

class AObject;

class NEXT_LIBRARY_EXPORT AMetaMethod {
public:
    typedef AVariant            (*InvokeMem)                (AObject *, int argc, const AVariant *);

    enum MethodType {
        Method                  = 0,
        Signal,
        Slot
    };

    struct Table {
        MethodType              type;
        const char             *name;
        InvokeMem               invoker;
        int                     argc;
        const AMetaType::Table **types;
    };

public:
    AMetaMethod                 (const Table *table);

    bool                        isValid                     () const;

    const char                 *name                        () const;
    string                      signature                   () const;

    MethodType                  type                        () const;
    AMetaType                   returnType                  () const;
    int                         parameterCount              () const;
    AMetaType                   parameterType               (int index) const;

    bool                        invoke                      (AObject *obj, AVariant &returnValue, int argc, const AVariant *args) const;

private:
    const Table                *m_pTable;

};

class AMethodCallEvent : public AEvent {
public:
    AMethodCallEvent            (int32_t method, AObject *senderm, const AVariant &args);

    AObject                    *sender                      () const;

    int32_t                     method                      () const;

    const AVariant             *args                        () const;

protected:
    AObject                    *m_pSender;

    int32_t                     m_Method;

    AVariant                    m_Args;
};

namespace unpack {
    template<unsigned...> struct indices {};

    template<unsigned N, unsigned... Is>
    struct indices_gen : indices_gen < N - 1, N - 1, Is... > {};

    template<unsigned... Is>
    struct indices_gen<0, Is...> : indices < Is... > {};
}

//Base template for function and methods invocation
template<typename Signature>
struct Invoker;

//Free functions
template<typename Return, typename... Args>
struct Invoker<Return(*)(Args...)> {
    // Workaround for the Visual Studio bug
    typedef Return(*Fun)(Args...);

    inline static int argCount() {
        return sizeof...(Args);
    }

    inline static const AMetaType::Table **types() {
        static const AMetaType::Table *staticTypes[] = {
            Table<Return>::get(),
            getTable<Args>()...
        };
        return staticTypes;
    }

    template<typename F, unsigned... Is>
    inline static AVariant invoke(AObject *obj, F f, const AVariant *args, unpack::indices<Is...>) {
        return f(args[Is]...); //any_cast<Args>(args[Is])...
    }

    template<Fun fun>
    static AVariant invoke(AObject *obj, int argc, const AVariant *args) {
        if(argc != sizeof...(Args)) {
            throw std::runtime_error("Bad argument count");
        }
        return invoke(obj, fun, args, unpack::indices_gen<sizeof...(Args)>());
    }
};

//Free function with no parameters
template<typename Return>
struct Invoker<Return(*)()> {
    typedef Return(*Fun)();

    inline static int argCount() {
        return 0;
    }

    inline static const AMetaType::Table **types() {
        static const AMetaType::Table *staticTypes[] = {
            Table<Return>::get()
        };
        return staticTypes;
    }

    template<typename F, unsigned... Is>
    inline static AVariant invoke(AObject *obj, F f, const AVariant *args, unpack::indices<Is...>) {
        return f(args[Is]...); // any_cast<Args>(args[Is])...
    }

    template<Fun fun>
    static AVariant invoke(AObject *obj, int argc, const AVariant *args) {
        if(argc != 0) {
            throw std::runtime_error("Bad argument count");
        }
        return invoke(obj, fun, args, unpack::indices_gen<0>());
    }
};

//Void free function
template<typename... Args>
struct Invoker<void(*)(Args...)> {
    typedef void(*Fun)(Args...);

    inline static int argCount() {
        return sizeof...(Args);
    }

    inline static const AMetaType::Table **types() {
        static const AMetaType::Table *staticTypes[] = {
            nullptr,
            getTable<Args>()...
        };
        return staticTypes;
    }

    template<typename F, unsigned... Is>
    inline static AVariant invoke(AObject *obj, F f, const AVariant *args, unpack::indices<Is...>) {
        f(args[Is]...); // any_cast<Args>(args[Is])...
        return AVariant();
    }

    template<Fun fun>
    static AVariant invoke(AObject *obj, int argc, const AVariant *args) {
        if(argc != sizeof...(Args)) {
            throw std::runtime_error("Bad argument count");
        }
        return invoke(obj, fun, args, unpack::indices_gen<sizeof...(Args)>());
    }
};

//Void free function with no arguments
template<>
struct Invoker<void(*)()> {
    typedef void(*Fun)();

    inline static int argCount() {
        return 0;
    }

    inline static const AMetaType::Table **types() {
        static const AMetaType::Table *staticTypes[] = {
            nullptr
        };
        return staticTypes;
    }

    template<unsigned... Is>
    inline static AVariant invoke(AObject *obj, Fun f, const AVariant *args, unpack::indices<Is...>) {
        f();
        return AVariant();
    }

    template<Fun fun>
    static AVariant invoke(AObject *obj, int argc, const AVariant *args) {
        if(argc != 0) {
            throw std::runtime_error("bad argument count");
        }
        return invoke(obj, fun, args, unpack::indices_gen<0>());
    }
};

//Base method template
template<typename Class, typename Return, typename... Args>
struct Invoker<Return(Class::*)(Args...)> {
    typedef Return(Class::*Fun)(Args...);

    inline static int argCount() {
        return sizeof...(Args);
    }

    inline static const AMetaType::Table **types() {
        static const AMetaType::Table *staticTypes[] = {
            Table<Return>::get(),
            getTable<Args>()...
        };
        return staticTypes;
    }

    template<typename F, unsigned... Is>
    inline static AVariant invoke(AObject *obj, F f, const AVariant *args, unpack::indices<Is...>) {
        return (static_cast<Class *>(obj)->*f)(args[Is]...); // any_cast<Args>(args[Is])...
    }

    template<Fun fun>
    static AVariant invoke(AObject *obj, int argc, const AVariant *args) {
        if(argc != sizeof...(Args)) {
            throw std::runtime_error("Bad argument count");
        }
        return invoke(obj, fun, args, unpack::indices_gen<sizeof...(Args)>());
    }
};

//Method with no parameters
template<typename Class, typename Return>
struct Invoker<Return(Class::*)()> {
    typedef Return(Class::*Fun)();

    inline static int argCount() {
        return 0;
    }

    inline static const AMetaType::Table **types() {
        static const AMetaType::Table *staticTypes[] = {
            Table<Return>::get()
        };
        return staticTypes;
    }

    template<typename F, unsigned... Is>
    inline static AVariant invoke(AObject *obj, F f, const AVariant *, unpack::indices<Is...>) {
        return (static_cast<Class *>(obj)->*f)();
    }

    template<Fun fun>
    static AVariant invoke(AObject *obj, int argc, const AVariant *args) {
        if(argc != 0) {
            throw std::runtime_error("Bad argument count");
        }
        return invoke(obj, fun, args, unpack::indices_gen<0>());
    }
};

//Void method
template<typename Class, typename... Args>
struct Invoker<void(Class::*)(Args...)> {
    typedef void(Class::*Fun)(Args...);

    inline static int argCount() {
        return sizeof...(Args);
    }

    inline static const AMetaType::Table **types() {
        static const AMetaType::Table *staticTypes[] = {
            nullptr,
            getTable<Args>("")... /// \todo: Set name
        };
        return staticTypes;
    }

    template<typename F, unsigned... Is>
    inline static AVariant invoke(AObject *obj, F f, const AVariant *args, unpack::indices<Is...>) {
        (static_cast<Class *>(obj)->*f)(args[Is].value<Args>()...);
        return AVariant();
    }

    template<Fun fun>
    static AVariant invoke(AObject *obj, int argc, const AVariant *args) {
        if (argc != sizeof...(Args)) {
            throw std::runtime_error("Bad argument count");
        }
        return invoke(obj, fun, args, unpack::indices_gen<sizeof...(Args)>());
    }
};

//Void method with no parameters
template<typename Class>
struct Invoker<void(Class::*)()> {
    typedef void(Class::*Fun)();

    inline static int argCount() {
        return 0;
    }

    inline static const AMetaType::Table **types() {
        static const AMetaType::Table *staticTypes[] = {
            nullptr
        };
        return staticTypes;
    }

    template<typename F, unsigned... Is>
    inline static AVariant invoke(AObject *obj, F f, const AVariant *args, unpack::indices<Is...>) {
        (static_cast<Class *>(obj)->*f)(args[Is]...); // any_cast<Args>(args[Is])...
        return AVariant();
    }

    template<Fun fun>
    static AVariant invoke(AObject *obj, int argc, const AVariant *args) {
        if (argc != 0) {
            throw std::runtime_error("Bad argument count");
        }
        return invoke(obj, fun, args, unpack::indices_gen<0>());
    }
};

//Base const method template
template<typename Class, typename Return, typename... Args>
struct Invoker<Return(Class::*)(Args...)const> {
    typedef Return(Class::*Fun)(Args...)const;

    inline static int argCount() {
        return sizeof...(Args);
    }

    inline static const AMetaType::Table **types() {
        static const AMetaType::Table *staticTypes[] = {
            Table<Return>::get(),
            getTable<Args>()...
        };
        return staticTypes;
    }

    template<typename F, unsigned... Is>
    inline static AVariant invoke(AObject *obj, F f, const AVariant *args, unpack::indices<Is...>) {
        return (const_cast<const Class *>(static_cast<Class *>(obj))->*f)(args[Is]...); // any_cast<Args>(args[Is])...
    }

    template<Fun fun>
    static AVariant invoke(AObject *obj, int argc, const AVariant *args) {
        if (argc != sizeof...(Args)) {
            throw std::runtime_error("Bad argument count");
        }
        return invoke(obj, fun, args, unpack::indices_gen<sizeof...(Args)>());
    }
};

//Const method with no parameters
template<typename Class, typename Return>
struct Invoker<Return(Class::*)()const> {
    typedef Return(Class::*Fun)()const;

    inline static int argCount() {
        return 0;
    }

    inline static const AMetaType::Table **types() {
        static const AMetaType::Table *staticTypes[] = {
            Table<Return>::get()
        };
        return staticTypes;
    }

    template<typename F, unsigned... Is>
    inline static AVariant invoke(AObject *obj, F f, const AVariant *, unpack::indices<Is...>) {
        return (const_cast<const Class *>(static_cast<Class *>(obj))->*f)();
    }

    template<Fun fun>
    static AVariant invoke(AObject *obj, int argc, const AVariant *args) {
        if (argc != 0) {
            throw std::runtime_error("Bad argument count");
        }
        return invoke(obj, fun, args, unpack::indices_gen<0>());
    }
};

//Void const method
template<typename Class, typename... Args>
struct Invoker<void(Class::*)(Args...)const> {
    typedef void(Class::*Fun)(Args...)const;

    inline static int argCount() {
        return sizeof...(Args);
    }

    template<typename T>
    inline static AMetaType::Table *getTable() {
        return Table<T>::get();
    }

    inline static const AMetaType::Table **types() {
        static const AMetaType::Table *staticTypes[] = {
            nullptr,
            getTable<Args>()...
        };
        return staticTypes;
    }

    template<typename F, unsigned... Is>
    inline static AVariant invoke(AObject *obj, F f, const AVariant *args, unpack::indices<Is...>) {
        (const_cast<const Class *>(static_cast<Class *>(obj))->*f)(args[Is]...); // any_cast<Args>(args[Is])...
        return AVariant();
    }

    template<Fun fun>
    static AVariant invoke(AObject *obj, int argc, const AVariant *args) {
        if(argc != sizeof...(Args)) {
            throw std::runtime_error("Bad argument count");
        }
        return invoke(obj, fun, args, unpack::indices_gen<sizeof...(Args)>());
    }
};

//Void const method with no parameters
template<typename Class>
struct Invoker<void(Class::*)()const> {
    typedef void(Class::*Fun)()const;

    inline static int argCount() {
        return 0;
    }

    inline static const AMetaType::Table **types() {
        static const AMetaType::Table *staticTypes[] = {
            nullptr
        };
        return staticTypes;
    }

    template<typename F, unsigned... Is>
    inline static AVariant invoke(AObject *obj, F f, const AVariant *args, unpack::indices<Is...>) {
        (const_cast<const Class *>(static_cast<Class *>(obj))->*f)(args[Is]...); // any_cast<Args>(args[Is])...
        return AVariant();
    }

    template<Fun fun>
    static AVariant invoke(AObject *obj, int argc, const AVariant *args) {
        if(argc != 0) {
            throw std::runtime_error("Bad argument count");
        }
        return invoke(obj, fun, args, unpack::indices_gen<0>());
    }
};

#endif // AMETAMETHOD_H
